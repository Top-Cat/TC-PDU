#ifndef config_h
#define config_h

#include <EEPROM.h>
#include <WString.h>
#include "output.h"

#define CONFIG_VERSION 8
#define MAX_OUTPUTS 8

struct RadiusConfig {
  IPAddress ip;
  uint16_t port;
  String secret;
  uint8_t timeout;
  uint8_t retries;
};

struct WifiConfig {
  String ssid;
  String password;
};

struct JWTConfig {
  uint32_t validityPeriod;
  String key;
};

struct NTPConfig {
  String host;
  String timezone;
};

struct LogConfig {
  uint64_t serialMask = 0;
  uint64_t syslogMask = 0;
  uint64_t emailMask = 0;

  String smtpServer = "";
  uint16_t smtpPort = 25;
  String smtpUser = "";
  String smtpPass = "";
  String smtpFrom = "";
  String smtpTo = "";

  uint8_t daysToKeep = 7;
};

struct MqttConfig {
  String host = "";
  uint16_t port = 1883;
  String clientId = "esp32-tcpdu";
  String username = "";
  String password = "";
  String prefix = "";
};

struct SyslogConfig {
  String host = "";
  uint16_t port = 5140;
};

class PDUConfig {
  public:
    void load();

    WifiConfig* getWifi();
    RadiusConfig* getRadius();
    JWTConfig* getJWT();
    NTPConfig* getNTP();
    LogConfig* getLog();
    MqttConfig* getMqtt();
    SyslogConfig* getSyslog();
    Output* getOutput(uint8_t idx);
    void storeOutputState(uint8_t idx, bool state);

    String adminPassword;

    void regenerateJWTKey();
    void save();
  private:
    WifiConfig wifi;
    RadiusConfig radius;
    JWTConfig jwt;
    NTPConfig ntp;
    LogConfig log;
    MqttConfig mqtt;
    SyslogConfig slog;
    Output outputs[MAX_OUTPUTS];
    EEPROMClass* OutputStates = new EEPROMClass("output-states");
};

extern PDUConfig config;

#endif
