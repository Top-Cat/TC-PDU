#include "network.h"

#include "config.h"
#include "logs/logs.h"
#include "esp_sntp.h"
#include "i2c.h"

char hostname[14];

void timeavailable(struct timeval *t) {
  bus.setTime(t->tv_sec);
}

void Network::task() {
  setupETH();
  setupAP();
  setupWifi();

  delay(1);

  sntp_set_time_sync_notification_cb(timeavailable);

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

  nextWifi = esp_timer_get_time() + RETRY_WIFI;

  while (true) {
    if (wifiReconfigured) {
      Serial.println(F("Wifi reconfigured"));

      wifiReconfigured = false;
      setupWifi();
    }

    if (!wifi && esp_timer_get_time() > nextWifi) {
      Serial.println(F("Wifi reconnect"));

      WiFi.reconnect();
      nextWifi = esp_timer_get_time() + RETRY_WIFI;
    }

    delay(500);
  }
}

bool Network::isConnected() {
  return eth || wifi;
}

void Network::reconfigureWifi() {
  wifiReconfigured = true;
}

void Network::setupWifi() {
  WifiConfig* wifiConf = config.getWifi();
  WiFi.mode(WIFI_AP_STA);

  if (wifiConf->enabled) {
    // Do nothing if wifi is enabled but not configured, continue broadcasting rescue ap
    if (wifiConf->ssid.length() > 0) {
      Serial.println(F("Attempting to connect to wifi"));

      esp_read_mac(wifimac, ESP_MAC_WIFI_STA);
      snprintf(hostname, sizeof(hostname), "tc-pdu-%02x%02x%02x", wifimac[3], wifimac[4], wifimac[5]);

      Serial.print(F("Hostname: "));
      Serial.println(hostname);

      WiFi.begin(wifiConf->ssid.c_str(), wifiConf->password.c_str());
    }
  } else {
    WiFi.disconnect(false, true);
  }
}

void Network::setupETH() {
  WiFi.onEvent(ethEventStatic);
  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
}

void Network::setupAP() {
  Serial.println("Setup AP");

  ap = true;
  WiFi.softAP(APssid, APpass);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

String Network::getMac() const {
  char macStr[7];
  snprintf(macStr, sizeof(macStr), "%02x%02x%02x", ethmac[3], ethmac[4], ethmac[5]);
  return macStr;
}

void Network::ethEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      ETH.macAddress(ethmac);
      snprintf(hostname, sizeof(hostname), "tc-pdu-%02x%02x%02x", ethmac[3], ethmac[4], ethmac[5]);

      ETH.setHostname(hostname);
      break;

    case ARDUINO_EVENT_ETH_CONNECTED:
      if (!ethConnected) {
        LogLine* msg = new LogLine();
        msg->type = NETWORK;
        snprintf(msg->message, sizeof(msg->message), "ETH connected");
        logger.msg(msg);

        WiFi.softAPdisconnect(true);
      }
      ethConnected = true;
      break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      if (!wifiConnected) {
        LogLine* msg = new LogLine();
        msg->type = NETWORK;
        snprintf(msg->message, sizeof(msg->message), "WiFi connected");
        logger.msg(msg);

        WiFi.softAPdisconnect(true);
      }
      wifiConnected = true;
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      {
        wifi = true;

        WifiConfig* wifiConfig = config.getWifi();
        LogLine* msg = new LogLine();
        msg->type = DEVICE_IP;
        snprintf(msg->message, sizeof(msg->message), "WiFi IP: %s, STA: %s", WiFi.localIP().toString(), wifiConfig->ssid);
        logger.msg(msg);
      }
      break;

    case ARDUINO_EVENT_ETH_GOT_IP:
      {
        eth = true;

        LogLine* msg = new LogLine();
        msg->type = DEVICE_IP;
        snprintf(msg->message, sizeof(msg->message), "ETH IP: %s (%s DUPLEX, %d Mbps)", ETH.localIP().toString(), ETH.fullDuplex() ? "FULL" : "HALF", ETH.linkSpeed());
        logger.msg(msg);
      }

      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      wifi = false;

      if (wifiConnected) {
        LogLine* msg = new LogLine();
        msg->type = NETWORK;
        snprintf(msg->message, sizeof(msg->message), "WiFi disconnected");
        logger.msg(msg);

        nextWifi = esp_timer_get_time() + RETRY_WIFI;
        if (!ethConnected) setupAP();
      }
      wifiConnected = false;
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      eth = false;

      if (ethConnected) {
        LogLine* msg = new LogLine();
        msg->type = NETWORK;
        snprintf(msg->message, sizeof(msg->message), "ETH disconnected");
        logger.msg(msg);

        if (!wifiConnected) setupAP();
      }
      ethConnected = false;
      break;

    case ARDUINO_EVENT_ETH_STOP:
      {
        eth = false;

        LogLine* msg = new LogLine();
        msg->type = NETWORK;
        snprintf(msg->message, sizeof(msg->message), "ETH stopped");
        logger.msg(msg);
      }
      break;
  }
}

String Network::getFormattedTime() const {
  struct tm ptm;
  getLocalTime(&ptm, 1000);

  char buffer[100];
  strftime(buffer, sizeof(buffer), "%F %T", &ptm);

  return buffer;
}

time_t Network::getEpochTime() const {
  struct tm timeinfo;
  getLocalTime(&timeinfo, 1000);
  return mktime(&timeinfo);
}

uint64_t Network::getEpochMs() const {
  struct timeval tv;
	gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

bool Network::hasTime() const {
  struct tm timeinfo;
  return getLocalTime(&timeinfo, 1000);
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
