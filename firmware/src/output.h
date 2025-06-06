#ifndef output_h
#define output_h

#include <stdint.h>
#include <WString.h>
#include <ArduinoJson.h>

enum class BootState { ON, LAST, OFF };
enum class OutputState { NORMAL, ALARM, TRIP };

extern bool coldBoot;

class Output {
  public:
    void serialize(uint8_t* ser);
    void deserialize(uint8_t* ser, uint8_t idx, bool state);
    void init();
    bool isDirty();

    char* getName();
    uint8_t getAddress();
    uint8_t getPriority();
    uint8_t getBootDelay();
    uint16_t getMaxPower();
    uint16_t getMinAlarm();
    uint16_t getMaxAlarm();
    BootState getBootState();
    OutputState getOutputState();

    float getVoltage();
    float getCurrent();
    float getVA();
    float getPower();
    float getKWH();

    void setFromJson(String user, JsonDocument* json);
    void setName(const char* _name);
    void setAddress(uint8_t _address);
    void setPriority(uint8_t _priority);
    void setBootDelay(uint8_t _bootDelay);
    void setMaxPower(uint16_t _maxPower);
    void setMinAlarm(uint16_t _minAlarm);
    void setMaxAlarm(uint16_t _maxAlarm);
    void setBootState(BootState _bootState);

    bool getState();
    void setState(const char* user, bool state);
    void tick(uint64_t time);

    void calibrateVoltage(float correction);
    void calibrateCurrent(float correction);
    void setVoltageCalibration(float correction);
    void setCurrentCalibration(float correction);

    void save();
  private:
    char name[64] = {};
    uint8_t address = 0xFF;
    uint8_t priority = 1;
    uint8_t bootDelay = 0;

    OutputState outputState = OutputState::NORMAL;
    uint16_t maxPower = 0;
    uint16_t minAlarm = 0;
    uint16_t maxAlarm = 0;

    BootState bootState = BootState::ON;
    float voltageCalibration = 1;
    float currentCalibration = 1;

    // Not saved
    bool dirty = false;
    uint8_t idx = 0;
    uint64_t onAt = 0;
    uint64_t lastTurnedOn = 0;
    String onAtUser = "";
    float lastPower = 0;
    bool lastState = false;
    bool relayState = false;
    void setState(const char* user, bool state, bool boot);

    // Calibrated values
    float VFC = 1;
    float CFC = 1;
    static void storeFloat(void* arr, float v);
    static void readFloat(void* arr, float* v);

    // Constant
    //uint32_t VolR1 = 1880000; // 4x 470k ohm
    //uint32_t VolR2 = 1000; // 1k ohm
    //float CurrentRF = 0.001; // 0.001 ohm
    static constexpr float VF = 1.88; // 1880000 / 1000
    static constexpr float CF = 1; // 1 / 1000R

    static float safeDiv(float lhs, uint32_t rhs);
    void setRelayState(const char* user, bool state);
    void handleAlarms(float power, uint64_t time);
};

#endif
