#include "i2c.h"

void ModuleBus::setRelay(uint8_t idx, bool on) {
  if (idx >= MAX_DEVICES) return;

  controlInfo[idx].relayState = on;
  controlUpdate[idx] = true;
}

void ModuleBus::setLed(uint8_t idx, bool red, bool green) {
  if (idx >= MAX_DEVICES) return;

  LedState newRed = red ? LedState::ON : LedState::OFF;
  LedState newGreen = green ? LedState::ON : LedState::OFF;

  if (controlInfo[idx].redLed == newRed && controlInfo[idx].greenLed == newGreen) return;

  controlInfo[idx].redLed = newRed;
  controlInfo[idx].greenLed = newGreen;
  controlUpdate[idx] = true;
}

void ModuleBus::setLed(uint8_t idx, LedState red, LedState green, float period, float dutyCycle) {
  if (idx >= MAX_DEVICES) return;

  controlInfo[idx].redLed = red;
  controlInfo[idx].greenLed = green;
  controlInfo[idx].period = period;
  controlInfo[idx].dutyCycle = dutyCycle;
  controlUpdate[idx] = true;
}

uint8_t ModuleBus::indexFor(uint8_t addr) {
  for (uint8_t idx = 0; idx < totalDevices; idx++) {
    if (addresses[idx] == addr) return idx;
  }
  return 0xFF;
}

void ModuleBus::parseBuffer(uint8_t buff[], PowerInfo *info) {
  info->VolPar = ((uint32_t)buff[2] <<16) + ((uint32_t)buff[3] <<8) + buff[4]; 
  if(bitRead(buff[20], 6) == 1) {
    info->VolData = ((uint32_t)buff[5]  <<16) + ((uint32_t)buff[6] <<8) + buff[7];
  }

  info->CurrentPar = ((uint32_t)buff[8]  <<16) + ((uint32_t)buff[9] <<8) + buff[10];  
  if(bitRead(buff[20], 5) == 1) {
    info->CurrentData = ((uint32_t)buff[11]  <<16) + ((uint32_t)buff[12] <<8) + buff[13];  
  }

  info->PowerPar = ((uint32_t)buff[14]  <<16) + ((uint32_t)buff[15] <<8) + buff[16];   
  if(bitRead(buff[20], 4) == 1) {
    info->PowerData = ((uint32_t)buff[17]  <<16) + ((uint32_t)buff[18] <<8) + buff[19];    
  }

  uint16_t PF = ((uint16_t)buff[21] <<8) + buff[22];
  uint32_t PFData = ((uint32_t)buff[26] << 24) + ((uint32_t)buff[25] << 16) + ((uint32_t)buff[26] << 8) + buff[27];

  info->TotalPulses = (PFData * 65536) + PF;
}

void ModuleBus::sendUpdates() {
  for (uint8_t idx = 0; idx < totalDevices; idx++) {
    if (controlUpdate[idx]) {
      ControlInfo info = controlInfo[idx];
      controlUpdate[idx] = false;
      uint8_t addr = addresses[idx];

      // 1.1 -> SLOW -> 1.1 * 64 -> 70, 35 (* 1024) -> 71680, 35840
      // 1 -> FAST -> 1 * 255 -> 255, 128 (* 256) -> 65280, 32640
      // 0.5 -> FAST -> 0.5 * 255 -> 128, 64 (* 256) -> 32640, 16320
      bool slowScale = info.period > 1;
      uint8_t multiple = slowScale ? 64 : 0xFF;
      uint8_t period = info.period * multiple;
      uint8_t dutyCycle = info.dutyCycle * period;

      // Debug i2c state
      /*Serial.print("Period IN = ");
      Serial.print(info.period);
      Serial.print(". Scale = ");
      Serial.print(slowScale ? "1024" : "256");
      Serial.print(", Period = ");
      Serial.print(period);
      Serial.print(", Duty Cycle = ");
      Serial.println(dutyCycle);*/

      Wire.beginTransmission(addr);
      Wire.write(
        (info.relayState<<0) |
        ((info.redLed != LedState::OFF ? 1 : 0)<<2) |
        ((info.greenLed != LedState::OFF ? 1 : 0)<<1) |
        ((info.redLed == LedState::FLASHING ? 1 : 0)<<3) |
        ((info.greenLed == LedState::FLASHING ? 1 : 0)<<4) |
        ((slowScale ? 1 : 0)<<5)
      );

      Wire.write(period);
      Wire.write(dutyCycle);
      Wire.endTransmission();
    }
  }
}

void ModuleBus::task() {
  Wire.begin(I2C_SDA, I2C_SCL, I2C_KHZ * 100000);
  uint8_t idx = 0;
  uint8_t buffer[28];
  uint8_t bidx = 0;

  for (uint8_t address = 0; address < 127; ++address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      addresses[idx++] = address;
    }
  }

  Serial.print("Found ");
  Serial.print(idx);
  Serial.println(" device(s)");
  totalDevices = idx;
  initComplete = true;
  while (true) {
    idx++;

    // Could use modulo but overflow may be whacky
    while (idx >= totalDevices) {
      idx -= totalDevices;
    }

    bidx = 0;
    Wire.requestFrom(addresses[idx], (uint8_t) 24);
    while (Wire.available()) {
      buffer[bidx++] = Wire.read();
    }

    parseBuffer(buffer, &powerInfo[idx]);

    // Wait between requesting power updates
    for (uint8_t wait = 0; wait < 30; wait++) {
      sendUpdates();
      delay(1);
    }
  }
}

///// Global object

ModuleBus bus;

///// Task

TaskHandle_t BusTask;

void busTask(void * parameter) {
  bus.task();
}

void ModuleBus::setupTask() {
  xTaskCreatePinnedToCore(busTask, "BusTask", 4096, NULL, 0, &BusTask, 0);
}
