#include "web.h"

#include "config.h"
#include "logs/logs.h"

PDUWeb::PDUWeb(WebServer* _server) {
  server = _server;
}

bool PDUWeb::deserializeOrError(WebServer* server, JsonDocument* doc) {
  String body = server->arg("plain");
  DeserializationError error = deserializeJson(*doc, body);

  if (error) {
    String output = F("deserializeJson() failed: ");
    output += error.f_str();
    server->send(500, textPlain, output);
    return false;
  }
  return true;
}

void PDUWeb::sendStaticHeaders() {
  server->sendHeader("Connection", "close");
}

void PDUWeb::setup() {
  jwt.setPSK(config.getJWT()->key);
  authEndpoints();
  configEndpoints();
  updateEndpoints();
  controlEndpoints();

  server->on("/logs", HTTP_GET, [&]() {
    JsonDocument doc;
    JsonArray devices = doc["logs"].to<JsonArray>();

    size_t rows = logger.getSize();
    doc["count"] = rows;

    LogLine arr[16];
    size_t actual = logger.readRows(arr, max(0, (uint8_t) rows - 16), 16);

    for (uint8_t idx = actual; idx > 0; idx--) {
      LogLine* line = &arr[idx - 1];

      JsonObject object = devices.add<JsonObject>();
      object["time"] = line->time;
      object["type"] = line->type;
      object["user"] = line->user;
      object["message"] = line->message;
    }

    String json;
    serializeJson(doc, json);

    sendStaticHeaders();
    server->send(200, "text/html", json);
  });

  const char* headerKeys[] = {"Cookie"};
  size_t headerKeysSize = sizeof(headerKeys) / sizeof(char*);
  server->collectHeaders(headerKeys, headerKeysSize);

  server->begin();
}

void PDUWeb::task() {
  server->handleClient();
}