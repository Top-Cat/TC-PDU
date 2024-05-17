#include "config.h"
#include "crypto/base64.h"

void PDUConfig::load() {
  EEPROM.begin(2048);
  OutputStates->begin(MAX_OUTPUTS);

  uint16_t addr = 0;
  uint16_t version = EEPROM.readUShort(addr);
  addr += 2;

  wifi.ssid = EEPROM.readString(addr);
  addr += wifi.ssid.length() + 1;

  wifi.password = EEPROM.readString(addr);
  addr += wifi.password.length() + 1;

  radius.ip = EEPROM.readUInt(addr);
  addr += 4;

  radius.port = EEPROM.readUShort(addr);
  addr += 2;

  radius.secret = EEPROM.readString(addr);
  addr += radius.secret.length() + 1;

  if (version >= 1) {
    radius.timeout = EEPROM.readUChar(addr++);
    radius.retries = EEPROM.readUChar(addr++);
  } else {
    radius.timeout = 5;
    radius.retries = 3;
  }

  adminPassword = EEPROM.readString(addr);
  addr += adminPassword.length() + 1;

  if (version >= 3) {
    jwt.validityPeriod = EEPROM.readUInt(addr);
    addr += 4;

    jwt.key = EEPROM.readString(addr);
    addr += jwt.key.length() + 1;
  } else {
    jwt.validityPeriod = 86400;
    regenerateJWTKey();

    adminPassword = "admin";
  }

  if (version >= 4) {
    ntp.host = EEPROM.readString(addr);
    addr += ntp.host.length() + 1;
  } else {
    ntp.host = "";
  }

  if (version >= 6) {
    ntp.timezone = EEPROM.readString(addr);
    addr += ntp.timezone.length() + 1;
  } else if (version >= 4) {
    ntp.timezone = "";
    addr += 4;
  } else {
    ntp.timezone = "";
  }

  if (version >= 6) {
    log.serialMask = EEPROM.readULong64(addr);
    addr += 8;

    log.syslogMask = EEPROM.readULong64(addr);
    addr += 8;

    log.emailMask = EEPROM.readULong64(addr);
    addr += 8;

    log.smtpServer = EEPROM.readString(addr);
    addr += log.smtpServer.length() + 1;

    log.smtpPort = EEPROM.readUShort(addr);
    addr += 2;

    log.smtpUser = EEPROM.readString(addr);
    addr += log.smtpUser.length() + 1;

    log.smtpPass = EEPROM.readString(addr);
    addr += log.smtpPass.length() + 1;

    log.smtpFrom = EEPROM.readString(addr);
    addr += log.smtpFrom.length() + 1;

    log.smtpTo = EEPROM.readString(addr);
    addr += log.smtpTo.length() + 1;

    log.daysToKeep = EEPROM.readByte(addr++);
  }

  if (version >= 7) {
    mqtt.host = EEPROM.readString(addr);
    addr += mqtt.host.length() + 1;

    mqtt.port = EEPROM.readUShort(addr);
    addr += 2;

    mqtt.username = EEPROM.readString(addr);
    addr += mqtt.username.length() + 1;

    mqtt.password = EEPROM.readString(addr);
    addr += mqtt.password.length() + 1;

    mqtt.clientId = EEPROM.readString(addr);
    addr += mqtt.clientId.length() + 1;

    mqtt.prefix = EEPROM.readString(addr);
    addr += mqtt.prefix.length() + 1;
  }

  if (version >= 8) {
    slog.host = EEPROM.readString(addr);
    addr += slog.host.length() + 1;

    slog.port = EEPROM.readUShort(addr);
    addr += 2;
  }

  uint8_t ser[128];
  for (uint8_t idx = 0; idx < MAX_OUTPUTS; idx++) {
    addr += EEPROM.readBytes(addr, ser, 128);

    bool state;
    if (version < 5) {
      state = EEPROM.readBool(addr++);
    } else {
      state = OutputStates->readBool(idx);
    }

    outputs[idx].deserialize(ser, idx, state);
  }

  if (version < CONFIG_VERSION) {
    save();
  }
}

WifiConfig* PDUConfig::getWifi() {
  return &wifi;
}

RadiusConfig* PDUConfig::getRadius() {
  return &radius;
}

JWTConfig* PDUConfig::getJWT() {
  return &jwt;
}

NTPConfig* PDUConfig::getNTP() {
  return &ntp;
}

LogConfig* PDUConfig::getLog() {
  return &log;
}

MqttConfig* PDUConfig::getMqtt() {
  return &mqtt;
}

SyslogConfig* PDUConfig::getSyslog() {
  return &slog;
}

void PDUConfig::regenerateJWTKey() {
  unsigned char rand[40];
  esp_fill_random(&rand, sizeof(rand));

  char psk[64];
  encode_base64(rand, sizeof(rand), (unsigned char*) &psk);

  jwt.key = String(psk);
}

Output* PDUConfig::getOutput(uint8_t idx) {
  if (idx >= MAX_OUTPUTS) return NULL;
  return &outputs[idx];
}

void PDUConfig::storeOutputState(uint8_t idx, bool state) {
  if (idx >= MAX_OUTPUTS) return;
  OutputStates->writeBool(idx, state);
  OutputStates->commit();
}

void PDUConfig::save() {
  uint16_t addr = 0;

  addr += EEPROM.writeUShort(addr, CONFIG_VERSION);
  addr += EEPROM.writeString(addr, wifi.ssid) + 1;
  addr += EEPROM.writeString(addr, wifi.password) + 1;

  addr += EEPROM.writeUInt(addr, radius.ip);
  addr += EEPROM.writeUShort(addr, radius.port);
  addr += EEPROM.writeString(addr, radius.secret) + 1;
  addr += EEPROM.writeUChar(addr, radius.timeout);
  addr += EEPROM.writeUChar(addr, radius.retries);

  addr += EEPROM.writeString(addr, adminPassword) + 1;

  addr += EEPROM.writeUInt(addr, jwt.validityPeriod);
  addr += EEPROM.writeString(addr, jwt.key) + 1;

  addr += EEPROM.writeString(addr, ntp.host) + 1;
  addr += EEPROM.writeString(addr, ntp.timezone) + 1;

  addr += EEPROM.writeULong64(addr, log.serialMask);
  addr += EEPROM.writeULong64(addr, log.syslogMask);
  addr += EEPROM.writeULong64(addr, log.emailMask);

  addr += EEPROM.writeString(addr, log.smtpServer) + 1;
  addr += EEPROM.writeUShort(addr, log.smtpPort);
  addr += EEPROM.writeString(addr, log.smtpUser) + 1;
  addr += EEPROM.writeString(addr, log.smtpPass) + 1;
  addr += EEPROM.writeString(addr, log.smtpFrom) + 1;
  addr += EEPROM.writeString(addr, log.smtpTo) + 1;
  addr += EEPROM.writeByte(addr, log.daysToKeep);

  addr += EEPROM.writeString(addr, mqtt.host) + 1;
  addr += EEPROM.writeUShort(addr, mqtt.port);
  addr += EEPROM.writeString(addr, mqtt.username) + 1;
  addr += EEPROM.writeString(addr, mqtt.password) + 1;
  addr += EEPROM.writeString(addr, mqtt.clientId) + 1;
  addr += EEPROM.writeString(addr, mqtt.prefix) + 1;

  addr += EEPROM.writeString(addr, slog.host) + 1;
  addr += EEPROM.writeUShort(addr, slog.port);

  for (uint8_t idx = 0; idx < MAX_OUTPUTS; idx++) {
    uint8_t ser[128];
    outputs[idx].serialize(ser);

    addr += EEPROM.writeBytes(addr, ser, 128);
  }
  EEPROM.commit();

  Serial.print("Saved ");
  Serial.print(addr);
  Serial.println(" bytes to EEPROM");
}

PDUConfig config;
