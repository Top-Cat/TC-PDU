#include "mqtt.h"
#include "network.h"
#include "config.h"
#include "control.h"

#include <ArduinoJson.h>

void PDUMqtt::onMqttConnect(bool sessionPresent) {
  MqttConfig* conf = config.getMqtt();

  // I tried (conf->prefix + "#").c_str() but it didn't work
  char result[prefix.length() + 1];
  strcpy(result, prefix.c_str());
  strcat(result, "#");

  mqttClient->subscribe(result, 2);

  connected = true;
}

void PDUMqtt::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  connected = false;
}

void PDUMqtt::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  MqttConfig* conf = config.getMqtt();
  const char* MQTTDelimiter = "/";

  if (strncmp(topic, prefix.c_str(), prefix.length()) != 0) return;
  // Prefix matches

  char* withoutPrefix = topic + prefix.length();
  char* token = strtok(withoutPrefix, MQTTDelimiter);

  uint8_t outputIdx = strtoul(token, NULL, 0);
  Output* output = config.getOutput(outputIdx);
  token = strtok(NULL, MQTTDelimiter);

  if (token != NULL && strncmp(token, "state", 5) == 0) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      output->setFromJson("MQTT", &doc);
    }
  }
}

void PDUMqtt::triggerChanges() {
  configChanges = true;
}

void PDUMqtt::registerCallbacks() {
  mqttClient->onConnect([this](bool s) { this->onMqttConnect(s); });
  mqttClient->onDisconnect([this](AsyncMqttClientDisconnectReason r) { this->onMqttDisconnect(r); });
  mqttClient->onMessage([this](char* t, char* p, AsyncMqttClientMessageProperties q, size_t l, size_t i, size_t u) { this->onMqttMessage(t, p, q, l, i, u); });
}

void PDUMqtt::task() {
  MqttConfig* conf = config.getMqtt();

  uint8_t backOff = 5;
  while (true) {
    while (!network.isConnected()) {
      backOff = 5;
      delay(100);
    }

    if (connected) {
      delay(5000);

      if (configChanges) {
        mqttClient->disconnect();
        continue;
      }

      JsonDocument doc;
      doc["power"] = control.getTotalPower();
      doc["mac"] = network.getMac();
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

      String topic = prefix + "state";
      mqttClient->publish(topic.c_str(), 0, false, json.c_str());
    } else {
      if (conf->host.length() > 0 && conf->enabled) {
        if (mqttClient == NULL) {
          mqttClient = new AsyncMqttClient();
          registerCallbacks();
        }

        // Setup config
        configChanges = false;
        mqttClient->setServer(conf->host.c_str(), conf->port);
        mqttClient->setClientId(conf->clientId.c_str());
        mqttClient->setCredentials(conf->username.c_str(), conf->password.c_str());
        prefix = conf->prefix + (conf->addMacToPrefix ? (network.getMac() + "/") : "") + "state";

        mqttClient->connect();
      } else if (mqttClient != NULL) {
        delete mqttClient;
        mqttClient = NULL;
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
