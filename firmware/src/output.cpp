#include "output.h"

#include "i2c.h"
#include "logs/logs.h"
#include "config.h"

#define ALARM_DELAY 6

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

  ser[78] = minAlarm >> 8;
  ser[79] = minAlarm & 0xFF;
  ser[80] = maxAlarm >> 8;
  ser[81] = maxAlarm & 0xFF;

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
    maxPower = 0;
    lastState = false;
    voltageCalibration = 1;
    currentCalibration = 1;
    minAlarm = 0;
    maxAlarm = 0;
  } else {
    strncpy(name, (char*) ser, sizeof(name));
    address = ser[64];
    priority = ser[65];
    bootDelay = ser[66];
    maxPower = (ser[67] << 8) | ser[68];
    bootState = (BootState) ser[69];

    readFloat(&ser[70], &voltageCalibration);
    readFloat(&ser[74], &currentCalibration);

    minAlarm = (ser[78] << 8) | ser[79];
    maxAlarm = (ser[80] << 8) | ser[81];

    lastState = state;
  }
}

void Output::init() {
  VFC = VF * voltageCalibration;
  CFC = CF * currentCalibration;

  // Ensure consistent state
  setAddress(address);

  // On at boot
  if (bootState == BootState::LAST) {
    setState(NULL, lastState, true);
  } else {
    setState(NULL, bootState == BootState::ON, true);
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

void Output::setMinAlarm(uint16_t _minAlarm) {
  minAlarm = _minAlarm;
  dirty = true;
}

uint16_t Output::getMinAlarm() {
  return minAlarm;
}

void Output::setMaxAlarm(uint16_t _maxAlarm) {
  maxAlarm = _maxAlarm;
  dirty = true;
}

uint16_t Output::getMaxAlarm() {
  return maxAlarm;
}

void Output::setBootState(BootState _bootState) {
  bootState = _bootState;
  dirty = true;
}

BootState Output::getBootState() {
  return bootState;
}

OutputState Output::getOutputState() {
  return outputState;
}

bool Output::getState() {
  return relayState;
}

void Output::setState(const char* user, bool state) {
  setState(user, state, false);
}

void Output::setState(const char* user, bool state, bool boot) {
  if (state && boot && bootDelay && !relayState) {
    // Ensure consistent state
    setRelayState(NULL, relayState);

    uint64_t time = esp_timer_get_time();
    onAt = time + ((uint64_t) bootDelay * 1000 * 1000);
    onAtUser = user;
    return;
  } else if (state && !relayState) {
    // If output is turned on manually cancel boot delay
    onAt = 0;
    onAtUser = "";
  }

  setRelayState(user, state);
}

void Output::setRelayState(const char* user, bool state) {
  bus.setRelay(idx, state);
  bus.setLed(idx, !state, state);

  if (idx < 16 && state != relayState) {
    relayState = state;

    if (state) lastTurnedOn = esp_timer_get_time();

    LogLine* msg = new LogLine();
    msg->type = OUTLET_STATE;
    if (user != NULL) strncpy(msg->user, user, 64);
    snprintf(msg->message, sizeof(msg->message), "%s was turned %s", name, state ? "ON" : "OFF");
    logger.msg(msg);

    config.storeOutputState(idx, state);
  }
}

void Output::setFromJson(String user, JsonDocument* doc) {
  JsonVariant state = (*doc)["state"];
  if (!state.isNull()) setState(user.c_str(), state);

  if ((*doc)["name"]) setName((*doc)["name"]);
  if ((*doc)["priority"]) setPriority((*doc)["priority"]);
  if ((*doc)["address"]) setAddress((*doc)["address"]);

  JsonVariant bootState = (*doc)["bootState"];
  if (!bootState.isNull()) setBootState((BootState)(uint8_t)bootState);

  JsonVariant bootDelay = (*doc)["bootDelay"];
  if (!bootDelay.isNull()) setBootDelay(bootDelay);

  JsonVariant maxPower = (*doc)["maxPower"];
  if (!maxPower.isNull()) setMaxPower(maxPower);

  JsonVariant minAlarm = (*doc)["minAlarm"];
  if (!minAlarm.isNull()) setMinAlarm(minAlarm);

  JsonVariant maxAlarm = (*doc)["maxAlarm"];
  if (!maxAlarm.isNull()) setMaxAlarm(maxAlarm);

  save();
}

void Output::save() {
  if (isDirty()) config.save();
}

void Output::handleAlarms(float power, uint64_t time) {
  bool bootstraped = time > lastTurnedOn && (time - lastTurnedOn) > (ALARM_DELAY * 1000000);
  bool alarmsValid = bootstraped && outputState != OutputState::ALARM;

  if (maxPower > 0 && power > maxPower) {
    if (!alarmsValid) return;

    outputState = OutputState::TRIP;

    LogLine* msg = new LogLine();
    msg->type = TRIP;
    snprintf(msg->message, sizeof(msg->message), "%s over max power (%.1f W)", name, power);
    logger.msg(msg);

    setRelayState(NULL, false);
  } else if (minAlarm > 0 && power < minAlarm) {
    if (!alarmsValid) return;

    LogLine* msg = new LogLine();
    msg->type = ALARM;
    snprintf(msg->message, sizeof(msg->message), "%s under min alarm power (%.1f W)", name, power);
    logger.msg(msg);

    outputState = OutputState::ALARM;
  } else if (maxAlarm > 0 && power > maxAlarm) {
    if (!alarmsValid) return;

    LogLine* msg = new LogLine();
    msg->type = ALARM;
    snprintf(msg->message, sizeof(msg->message), "%s over max alarm power (%.1f W)", name, power);
    logger.msg(msg);

    outputState = OutputState::ALARM;
  } else {
    outputState = OutputState::NORMAL;
  }
}

void Output::tick(uint64_t time) {
  if (onAt > 0 && time > onAt) {
    onAt = 0;
    setRelayState(onAtUser.c_str(), true);
    onAtUser = "";
  }

  if (relayState) {
    float power = getPower();
    handleAlarms(power, time);

    if (abs(lastPower - power) > 20) {
      lastPower = power;
      bus.setLed(idx, LedState::OFF, LedState::FLASHING, 100 / power, 0.5);
    }
  } else {
    bus.setLed(idx, true, false);
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
  setVoltageCalibration(voltageCalibration * correction);
}

void Output::calibrateCurrent(float correction) {
  setCurrentCalibration(currentCalibration * correction);
}

void Output::setVoltageCalibration(float newVC) {
  dirty |= voltageCalibration - newVC > 0.0001;
  voltageCalibration = newVC;
  VFC = VF * voltageCalibration;
}

void Output::setCurrentCalibration(float newCC) {
  dirty |= currentCalibration - newCC > 0.0001;
  currentCalibration = newCC;
  CFC = CF * currentCalibration;
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
