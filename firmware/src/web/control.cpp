#include "web.h"
#include "config.h"
#include "control.h"
#include "network.h"

void PDUWeb::controlEndpoints() {
  server->on("/state", HTTP_GET, [&]() {
    JsonDocument doc;
    doc["power"] = control.getTotalPower();
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
    }

    String json;
    serializeJson(doc, json);

    sendStaticHeaders();
    server->send(200, "text/html", json);
  });

  server->on("/system", HTTP_GET, [&]() {
    JsonDocument doc;
    doc["power"] = control.getTotalPower();
    doc["time"] = network.getFormattedTime();
    doc["fw"] = "0.0.1";
    doc["uptime"] = control.getUptime();

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

    String json;
    serializeJson(doc, json);

    sendStaticHeaders();
    server->send(200, "text/html", json);
  });

  server->on("/state", HTTP_POST, [&]() {
    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    uint8_t idx = doc["idx"];
    Output* output = config.getOutput(idx);

    JsonVariant state = doc["state"];
    if (!state.isNull()) output->setState(state);

    if (doc["name"]) output->setName(doc["name"]);
    if (doc["priority"]) output->setPriority(doc["priority"]);
    if (doc["address"]) output->setAddress(doc["address"]);

    JsonVariant bootState = doc["bootState"];
    if (!bootState.isNull()) output->setBootState((BootState)(uint8_t)bootState);

    JsonVariant bootDelay = doc["bootDelay"];
    if (!bootDelay.isNull()) output->setBootDelay(bootDelay);

    JsonVariant maxPower = doc["maxPower"];
    if (!maxPower.isNull()) output->setMaxPower(maxPower);

    if (output->isDirty()) config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  // TODO: Calibration endpoint / allow calibration via /state
}