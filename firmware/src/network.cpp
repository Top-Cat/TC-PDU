#include "network.h"

#include "config.h"
#include "logs/logs.h"

void Network::task() {
  disconnected();
  setupETH();
  setupWifi();

  delay(1);

  NTPConfig* ntpConf = config.getNTP();
  if (ntpConf->host.length() > 0) {
    timeClient.setPoolServerName(ntpConf->host.c_str());
  }
  timeClient.setTimeOffset(ntpConf->offset);
  timeClient.begin();

  setupComplete = true;

  while (true) {
    if (!ap && !wifi && !eth && (esp_timer_get_time() - lastConnected) >= TIMEOUT) {
      Serial.println();
      Serial.println(F("Timeout waiting for connection"));
      
      setupAP();
    }

    if (ap && eth) {
      setupWifi();
    }

    if (wifi || eth) {
      timeClient.setTimeOffset(ntpConf->offset);
      timeClient.update();
    }

    delay(500);
  }
}

void Network::setupWifi() {
  ap = false;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  WiFi.setHostname("TC-pdu-wifi");

  WifiConfig* wifiConf = config.getWifi();
  if (wifiConf->ssid.length() > 0) {
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
      Serial.println(F("ETH Connected"));
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
      Serial.println(F("Lost wifi connection"));
      if (wifi) disconnected();
      wifi = false;
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      if (eth) disconnected();
      eth = false;
      break;

    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("\nETH Stopped");
      if (eth) disconnected();
      eth = false;
      break;
  }
}

String Network::getFormattedTime() const {
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t*) &epochTime);

  char buffer[100];
  strftime(buffer, sizeof(buffer), "%F %T", ptm);

  return buffer;
}

time_t Network::getEpochTime() const {
  return timeClient.getEpochTime();
}

bool Network::hasTime() const {
  return timeClient.isTimeSet();
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
