#ifndef mqtt_h
#define mqtt_h

#include <AsyncMqttClient.h>

class PDUMqtt {
  public:
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    //void onMqttSubscribe(uint16_t packetId, uint8_t qos);
    //void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    //void onMqttPublish(uint16_t packetId);

    static void setupTask();
    void task();
    void triggerChanges();
  private:
    bool configChanges = false;
    void registerCallbacks();
    AsyncMqttClient* mqttClient;
    bool connected;
};

extern PDUMqtt mqtt;

#endif
