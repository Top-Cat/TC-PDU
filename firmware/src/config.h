#ifndef config_h
#define config_h

#include <EEPROM.h>
#include <WString.h>
#include "output.h"

#define CONFIG_VERSION 6
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
  uint64_t serialMask;
  uint64_t syslogMask;
  uint64_t emailMask;

  String smtpServer;
  uint16_t smtpPort;
  String smtpUser;
  String smtpPass;
  String smtpFrom;
  String smtpTo;

  uint8_t daysToKeep;
};

class PDUConfig {
  public:
    void load();

    WifiConfig* getWifi();
    RadiusConfig* getRadius();
    JWTConfig* getJWT();
    NTPConfig* getNTP();
    LogConfig* getLog();
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
    Output outputs[MAX_OUTPUTS];
    EEPROMClass* OutputStates = new EEPROMClass("output-states");
};

extern PDUConfig config;

#endif
