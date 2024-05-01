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

    ntp.offset = EEPROM.readUInt(addr);
    addr += 4;
  } else {
    ntp.host = "";
    ntp.offset = 0;
  }

  // TODO: Store in EEPROM (Currently easier to test like this)
  // Only use serial for now
  log.emailMask = 0;
  log.serialMask = 0xFFFFFFFFFFFFFFFF;
  log.syslogMask = 0;
  log.smtpServer = "mail.thomasc.co.uk";
  log.smtpPort = 25;
  log.smtpUser = "";
  log.smtpPass = "";
  log.smtpFrom = "pdu@thomasc.co.uk";
  log.smtpTo = "test@thomasc.co.uk";

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
  addr += EEPROM.writeUInt(addr, ntp.offset);

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
