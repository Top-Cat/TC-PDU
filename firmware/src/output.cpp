#include "output.h"

#include "i2c.h"
#include "logs/logs.h"
#include "config.h"

void Output::serialize(uint8_t* ser) {
  strncpy((char*) ser, name, sizeof(name));
  ser[64] = address;
  ser[65] = priority;
  ser[66] = bootDelay;
  ser[67] = maxPower >> 8;
  ser[68] = maxPower & 0xFF;
  ser[69] = (uint8_t) bootState;

  storeFloat(&ser[70], voltageCalibration);
  storeFloat(&ser[74], currentCalibration);

  dirty = false;
}

void Output::deserialize(uint8_t* ser, uint8_t idx, bool state) {
  bool null = false;
  for (uint8_t i = 0; i < sizeof(name); i++) {
    if (ser[i] == 0) {
      null = true;
      break;
    }
  }

  if (!null) {
    snprintf(name, sizeof(name), "Output %d", idx + 1);
    address = 0xFF;
    priority = 1;
    bootDelay = 0;
    maxPower = 1000;
    lastState = false;
    voltageCalibration = 1;
    currentCalibration = 1;
  } else {
    strncpy(name, (char*) ser, sizeof(name));
    address = ser[64];
    priority = ser[65];
    bootDelay = ser[66];
    maxPower = (ser[67] << 8) | ser[68];
    bootState = (BootState) ser[69];

    readFloat(&ser[70], &voltageCalibration);
    readFloat(&ser[74], &currentCalibration);

    lastState = state;
  }
}

void Output::init() {
  VFC = VF * voltageCalibration;
  CFC = CF * currentCalibration;

  // Ensure consistent state
  setAddress(address);

  auto reason = esp_reset_reason();

  // On at boot
  if (bootState == BootState::LAST && reason != ESP_RST_BROWNOUT) {
    setState(lastState);
  } else {
    setState(bootState == BootState::ON);
  }
}

char* Output::getName() {
  return name;
}

uint8_t Output::getAddress() {
  return address;
}

void Output::setName(const char *_name) {
  strncpy(name, _name, sizeof(name));
  dirty = true;
}

void Output::setAddress(uint8_t _address) {
  address = _address;
  idx = bus.indexFor(address);
  dirty = true;
}

void Output::setPriority(uint8_t _priority) {
  priority = _priority;
  dirty = true;
}

uint8_t Output::getPriority() {
  return priority;
}

void Output::setBootDelay(uint8_t _bootDelay) {
  bootDelay = _bootDelay;
  dirty = true;
}

uint8_t Output::getBootDelay() {
  return bootDelay;
}

void Output::setMaxPower(uint16_t _maxPower) {
  maxPower = _maxPower;
  dirty = true;
}

uint16_t Output::getMaxPower() {
  return maxPower;
}

void Output::setBootState(BootState _bootState) {
  bootState = _bootState;
  dirty = true;
}

BootState Output::getBootState() {
  return bootState;
}

bool Output::getState() {
  return relayState;
}

void Output::setState(bool state) {
  if (state && bootDelay && !relayState) {
    // Ensure consistent state
    setRelayState(relayState);
    uint64_t time = esp_timer_get_time();
    onAt = time + ((uint64_t) bootDelay * 1000 * 1000);
    return;
  }

  setRelayState(state);
}

void Output::setRelayState(bool state) {
  bus.setRelay(idx, state);
  bus.setLed(idx, !state, state);

  if (idx < 16 && state != relayState) {
    relayState = state;

    LogLine* msg = new LogLine();
    msg->type = OUTLET_STATE;
    snprintf(msg->message, sizeof(msg->message), "%s was turned %s", name, state ? "ON" : "OFF");
    config.storeOutputState(idx, state);
    logger.msg(msg);
  }
}

void Output::tick(uint64_t time) {
  if (onAt > 0 && time > onAt) {
    onAt = 0;
    setRelayState(true);
  } else if (!relayState) {
    bus.setLed(idx, true, false);
  } else {
    float power = getPower();
    if (abs(lastPower - power) > 20) {
      lastPower = power;
      bus.setLed(idx, LedState::OFF, LedState::FLASHING, 100 / power, 0.5);
    }
  }
}

float Output::getVoltage() {
  if (idx >= 16) return 0;

  PowerInfo* powerData = &bus.powerInfo[idx];
  return safeDiv(powerData->VolPar * VFC, powerData->VolData);
}

float Output::getCurrent() {
  if (idx >= 16) return 0;

  PowerInfo* powerData = &bus.powerInfo[idx];
  return safeDiv(powerData->CurrentPar * CFC, powerData->CurrentData);
}

float Output::getVA() {
  if (idx >= 16) return 0;

  PowerInfo* powerData = &bus.powerInfo[idx];
  float v = safeDiv(powerData->VolPar * VFC, powerData->VolData);
  float a = safeDiv(powerData->CurrentPar * CFC, powerData->CurrentData);
  return v * a;
}

float Output::getPower() {
  if (idx >= 16) return 0;

  PowerInfo* powerData = &bus.powerInfo[idx];
  return safeDiv(powerData->PowerPar * VFC * CFC, powerData->PowerData);
}

float Output::getKWH() {
  if (idx >= 16) return 0;

  PowerInfo* powerData = &bus.powerInfo[idx];
  float pulsesPerKWH = (1/(float)powerData->PowerPar) * (1/(VFC * CFC)) * 3600000000000;
  return powerData->TotalPulses / pulsesPerKWH;
}

float Output::safeDiv(float lhs, uint32_t rhs) {
    if ( rhs == 0 ) {
      return 0;
    } else {
      return lhs/rhs;
    }
}

bool Output::isDirty() {
  return dirty;
}

void Output::calibrateVoltage(float correction) {
  voltageCalibration *= correction;
  VFC = VF * voltageCalibration;
  dirty |= correction != 1;
}

void Output::calibrateCurrent(float correction) {
  currentCalibration *= correction;
  CFC = CF * currentCalibration;
  dirty |= correction != 1;
}

void Output::storeFloat(void* arr, float v) {
  union {
    float float_variable;
    byte temp_array[4];
  } f2b;

  f2b.float_variable = v;
  memcpy(arr, f2b.temp_array, 4);
}

void Output::readFloat(void* arr, float* v) {
  union {
    float float_variable;
    byte temp_array[4];
  } b2f;

  memcpy(b2f.temp_array, arr, 4);
  *v = b2f.float_variable;
}
