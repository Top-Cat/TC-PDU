#include "network.h"

#include "config.h"
#include "logs/logs.h"

void Network::task() {
  disconnected();
  setupETH();
  setupWifi();

  delay(1);

  NTPConfig* ntpConf = config.getNTP();
  const char* tz = "GMT0BST,M3.5.0/1,M10.5.0";
  const char* ntpHost = "pool.ntp.org";
  if (ntpConf->host.length() > 0) {
    ntpHost = ntpConf->host.c_str();
  }
  if (ntpConf->timezone.length() > 0) {
    tz = ntpConf->timezone.c_str();
  }
  configTzTime(tz, ntpHost);

  setupComplete = true;

  LogLine* msg = new LogLine();
  msg->type = BOOT;
  snprintf(msg->message, sizeof(msg->message), "Network setup complete");
  logger.msg(msg);

  while (true) {
    if (!ap && !wifi && !eth && (esp_timer_get_time() - lastConnected) >= TIMEOUT) {
      //Serial.println();
      Serial.println(F("Timeout waiting for connection"));

      setupAP();
    }

    if ((ap && eth) || wifiReconfigured) {
      wifiReconfigured = false;
      setupWifi();
    }

    delay(500);
  }
}

void Network::reconfigureWifi() {
  wifiReconfigured = true;
}

void Network::setupWifi() {
  ap = false;

  WifiConfig* wifiConf = config.getWifi();
  WiFi.mode(wifiConf->enabled ? WIFI_STA : WIFI_OFF);
  WiFi.disconnect();

  WiFi.setHostname("TC-pdu-wifi");

  if (wifiConf->ssid.length() > 0 && wifiConf->enabled) {
    WiFi.begin(wifiConf->ssid.c_str(), wifiConf->password.c_str());
  }
}

void Network::setupETH() {
  WiFi.onEvent(ethEventStatic);
  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
}

void Network::setupAP() {
  Serial.println("Setup AP");

  WiFi.mode(WIFI_AP);
  WiFi.disconnect();
  ap = true;
  WiFi.softAP(APssid, APpass);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void Network::disconnected() {
  lastConnected = esp_timer_get_time();
}

void Network::ethEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      ETH.setHostname("TC-pdu");
      break;

    case ARDUINO_EVENT_ETH_CONNECTED:
      //Serial.println(F("ETH Connected"));
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      if (!wifi) {
        wifi = true;

        WifiConfig* wifiConfig = config.getWifi();
        LogLine* msg = new LogLine();
        msg->type = DEVICE_IP;
        snprintf(msg->message, sizeof(msg->message), "WiFi IP: %s, STA: %s", WiFi.localIP().toString(), wifiConfig->ssid);
        logger.msg(msg);
      }
      break;

    case ARDUINO_EVENT_ETH_GOT_IP:
      if (!eth) {
        eth = true;

        LogLine* msg = new LogLine();
        msg->type = DEVICE_IP;
        snprintf(msg->message, sizeof(msg->message), "ETH IP: %s (%s DUPLEX, %d Mbps)", ETH.localIP().toString(), ETH.fullDuplex() ? "FULL" : "HALF", ETH.linkSpeed());
        logger.msg(msg);
      }

      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      //Serial.println(F("Lost wifi connection"));
      if (wifi) disconnected();
      wifi = false;
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      //Serial.println("ETH Disconnected");
      if (eth) disconnected();
      eth = false;
      break;

    case ARDUINO_EVENT_ETH_STOP:
      //Serial.println("\nETH Stopped");
      if (eth) disconnected();
      eth = false;
      break;
  }
}

String Network::getFormattedTime() const {
  struct tm ptm;
  getLocalTime(&ptm);

  char buffer[100];
  strftime(buffer, sizeof(buffer), "%F %T", &ptm);

  return buffer;
}

time_t Network::getEpochTime() const {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  return mktime(&timeinfo);
}

uint64_t Network::getEpochMs() const {
  struct timeval tv;
	gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

bool Network::hasTime() const {
  struct tm timeinfo;
  return getLocalTime(&timeinfo);
}

uint16_t Network::getOffset() const {
    time_t t, t2;
    struct tm *tm2;

    time(&t);
    tm2 = gmtime(&t);
    tm2->tm_isdst = -1;
    t2 = mktime(tm2);
    return t - t2;
}

WifiState Network::wifiState() const {
  WifiState state;
  state.connected = wifi;
  state.mac = WiFi.macAddress();
  state.ip = WiFi.localIP().toString();
  state.dns = WiFi.dnsIP().toString();
  state.gateway = WiFi.gatewayIP().toString();
  state.subnet = WiFi.subnetMask().toString();
  state.rssi = WiFi.RSSI();

  return state;
}

EthState Network::ethState() const {
  EthState state;
  state.connected = eth;
  state.mac = ETH.macAddress();
  state.ip = ETH.localIP().toString();
  state.dns = ETH.dnsIP().toString();
  state.gateway = ETH.gatewayIP().toString();
  state.subnet = ETH.subnetMask().toString();
  state.fullDuplex = ETH.fullDuplex();
  state.linkSpeed = ETH.linkSpeed();

  return state;
}

///// Global object

WiFiUDP udpClient;
Network network;

void Network::ethEventStatic(WiFiEvent_t event) {
  network.ethEvent(event);
}

///// Task

TaskHandle_t NetworkTask;

void networkTask(void * parameter) {
  network.task();
}

void Network::setupTask() {
  xTaskCreatePinnedToCore(networkTask, "NetTask", 4096, NULL, 0, &NetworkTask, 0);
}
