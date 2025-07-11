#ifndef i2c_h
#define i2c_h

#include <stdint.h>
#include <Wire.h>
#include "uRTCLib.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>

#define ONE_WIRE_BUS 15

#define MAX_DEVICES 16
#define I2C_SDA 33
#define I2C_SCL 32
#define I2C_KHZ 4

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

struct PowerInfo {
  uint8_t StateReg = 0;
  uint32_t VolPar = 0;
  uint32_t VolData = 0;
  uint32_t CurrentPar = 0;
  uint32_t CurrentData = 0;
  uint32_t PowerPar = 0;
  uint32_t PowerData = 0;
  uint64_t TotalPulses = 0;
};

enum class LedState { ON, FLASHING, OFF };

struct ControlInfo {
  bool relayState;
  LedState redLed;
  LedState greenLed;
  float period;
  float dutyCycle;
};

class ModuleBus {
  public:
    uint8_t indexFor(uint8_t addr);
    uint8_t* getAddressList();
    static void setupTask();
    void task();

    float* getReadings();
    void updateTemp(uint8_t index, float val);

    void setRelay(uint8_t idx, bool on);
    void setLed(uint8_t idx, LedState red, LedState green, float period, float dutyCycle);
    void setLed(uint8_t idx, bool red, bool green);
    void setTime(time_t t);

    bool initComplete = false;
    bool hasRtc = false;

    PowerInfo powerInfo[MAX_DEVICES];
    uint8_t totalDevices = 0;
  private:
    uRTCLib* rtc = new uRTCLib(0x68);
    OneWire* oneWire = new OneWire(ONE_WIRE_BUS);
    DallasTemperature* dt = new DallasTemperature(oneWire);
    NonBlockingDallas* sensors = new NonBlockingDallas(dt);

    float temps[3] = {0};

    ControlInfo controlInfo[MAX_DEVICES] = {};
    bool controlUpdate[MAX_DEVICES] = {false};
    uint8_t addresses[MAX_DEVICES] = {0};

    void parseBuffer(uint8_t buff[], PowerInfo *info, bool isOn);
    void sendUpdates();
};

extern ModuleBus bus;

#endif
