#include "web.h"
#include "config.h"
#include "control.h"
#include "i2c.h"
#include "network.h"
#include "version.h"
#include "frequency.h"

#include <SPIFFS.h>

void PDUWeb::controlEndpoints() {
  server->on("/api/state", HTTP_GET, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    doc["power"] = control.getTotalPower();
    doc["frequency"] = freq.getFrequency();
    JsonArray devices = doc["devices"].to<JsonArray>();

    for (uint8_t idx = 0; idx < MAX_OUTPUTS; idx++) {
      Output* output = config.getOutput(idx);

      JsonObject object = devices.add<JsonObject>();
      object["name"] = output->getName();
      object["voltage"] = output->getVoltage();
      object["current"] = output->getCurrent();
      object["power"] = output->getPower();
      object["kwh"] = output->getKWH();
      object["va"] = output->getVA();
      object["address"] = output->getAddress();
      object["state"] = output->getState();
      object["bootState"] = (uint8_t) output->getBootState();
      object["bootDelay"] = output->getBootDelay();
      object["priority"] = output->getPriority();
      object["maxPower"] = output->getMaxPower();
      object["minAlarm"] = output->getMinAlarm();
      object["maxAlarm"] = output->getMaxAlarm();
      object["outputState"] = (uint8_t) output->getOutputState();
    }

    String json;
    serializeJson(doc, json);

    sendStaticHeaders();
    server->send(200, "text/html", json);
  });

  server->on("/api/i2c", HTTP_GET, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    doc["count"] = bus.totalDevices;
    JsonArray devices = doc["devices"].to<JsonArray>();

    uint8_t* addresses = bus.getAddressList();
    for (uint8_t idx = 0; idx < bus.totalDevices; idx++) {
      devices.add(addresses[idx]);
    }

    String json;
    serializeJson(doc, json);

    sendStaticHeaders();
    server->send(200, "text/html", json);
  });

  server->on("/api/system", HTTP_GET, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    doc["power"] = control.getTotalPower();
    doc["time"] = network.getFormattedTime();
    doc["timeMs"] = network.getEpochMs();
    doc["offset"] = network.getOffset();
    doc["fw"] = "0.0." STRING(BUILD_NUMBER) STRING(SNAPSHOT);
    doc["uptime"] = control.getUptime();
    doc["mem"] = esp_get_free_heap_size();
    doc["rtc"] = bus.hasRtc;

    doc["fs"]["used"] = SPIFFS.usedBytes();
    doc["fs"]["total"] = SPIFFS.totalBytes();

    WifiState wState = network.wifiState();
    doc["wifi"]["connected"] = wState.connected;
    doc["wifi"]["mac"] = wState.mac;
    if (wState.connected) {
      doc["wifi"]["ip"] = wState.ip;
      doc["wifi"]["dns"] = wState.dns;
      doc["wifi"]["gateway"] = wState.gateway;
      doc["wifi"]["subnet"] = wState.subnet;
      doc["wifi"]["rssi"] = wState.rssi;
    }

    EthState eState = network.ethState();
    doc["eth"]["connected"] = eState.connected;
    doc["eth"]["mac"] = eState.mac;
    if (eState.connected) {
      doc["eth"]["ip"] = eState.ip;
      doc["eth"]["dns"] = eState.dns;
      doc["eth"]["gateway"] = eState.gateway;
      doc["eth"]["subnet"] = eState.subnet;
      doc["eth"]["fullDuplex"] = eState.fullDuplex;
      doc["eth"]["linkSpeed"] = eState.linkSpeed;
    }

    JsonArray temps = doc["temps"].to<JsonArray>();

    float* tempArr = bus.getReadings();
    for (uint8_t idx = 0; idx < 3; idx++) {
      temps.add(tempArr[idx]);
    }

    String json;
    serializeJson(doc, json);

    sendStaticHeaders();
    server->send(200, "text/html", json);
  });

  server->on("/api/state", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    uint8_t idx = doc["idx"];
    Output* output = config.getOutput(idx);

    output->setFromJson(user, &doc);

    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/reboot", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    ESP.restart();

    sendStaticHeaders();
    server->send(200, textPlain, "REBOOTING");
  }, [&]() { });

  server->on("/api/calibrate", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    uint8_t idx = doc["idx"];
    Output* output = config.getOutput(idx);

    if (doc["current"]) output->calibrateCurrent(doc["current"]);
    if (doc["voltage"]) output->calibrateVoltage(doc["voltage"]);

    if (doc["currentDirect"]) output->setCurrentCalibration(doc["currentDirect"]);
    if (doc["voltageDirect"]) output->setVoltageCalibration(doc["voltageDirect"]);

    output->save();

    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });
}
