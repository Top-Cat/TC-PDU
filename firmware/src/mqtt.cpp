#include "mqtt.h"
#include "network.h"
#include "config.h"
#include "control.h"

#include <ArduinoJson.h>

void PDUMqtt::onMqttConnect(bool sessionPresent) {
  // TODO: Subscribe to control topics
  //uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  connected = true;
}

void PDUMqtt::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  connected = false;
}

void PDUMqtt::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  // TODO: Handle control messages
}

void PDUMqtt::task() {
  MqttConfig* conf = config.getMqtt();

  // Register callbacks
  mqttClient.onConnect([this](bool s) { this->onMqttConnect(s); });
  mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason r) { this->onMqttDisconnect(r); });
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage([this](char* t, char* p, AsyncMqttClientMessageProperties q, size_t l, size_t i, size_t u) { this->onMqttMessage(t, p, q, l, i, u); });
  //mqttClient.onPublish(onMqttPublish);

  uint8_t backOff = 5;
  while (true) {
    while (!network.setupComplete) {
      backOff = 5;
      delay(100);
    }

    // TODO: Detect config changes
    if (connected) {
      delay(5000);

      JsonDocument doc;
      doc["power"] = control.getTotalPower();
      JsonArray devices = doc["devices"].to<JsonArray>();

      for (uint8_t idx = 0; idx < MAX_OUTPUTS; idx++) {
        Output* output = config.getOutput(idx);

        JsonObject object = devices.add<JsonObject>();
        object["index"] = idx;
        object["name"] = output->getName();
        object["voltage"] = output->getVoltage();
        object["current"] = output->getCurrent();
        object["power"] = output->getPower();
        object["kwh"] = output->getKWH();
        object["va"] = output->getVA();
        object["state"] = output->getState();
      }

      String json;
      serializeJson(doc, json);

      String topic = conf->prefix + "state";
      mqttClient.publish(topic.c_str(), 0, false, json.c_str());
    } else {
      if (conf->host.length() > 0) {
        // Setup config
        mqttClient.setServer(conf->host.c_str(), conf->port);
        mqttClient.setClientId(conf->clientId.c_str());
        mqttClient.setCredentials(conf->username.c_str(), conf->password.c_str());

        mqttClient.connect();
      }

      delay(backOff * 1000);
      if (backOff < 60) backOff += 5;
    }
  }
}

///// Global object

PDUMqtt mqtt;

///// Task

TaskHandle_t MqttTask;

void mqttTask(void * parameter) {
  mqtt.task();
}

void PDUMqtt::setupTask() {
  xTaskCreatePinnedToCore(mqttTask, "MqttTask", 4096, NULL, 0, &MqttTask, 0);
}
