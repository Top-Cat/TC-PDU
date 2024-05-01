#include "web.h"
#include <uri/UriBraces.h>

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

  server->on(UriBraces("/api/logs/{}"), HTTP_GET, [&]() {

    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    JsonArray devices = doc["logs"].to<JsonArray>();

    size_t rows = logger.getSize();
    uint8_t pageVar = strtoul(server->pathArg(0).c_str(), NULL, 0);
    uint8_t maxPage = (rows / pageSize) + 1;
    uint8_t page = min(max((uint8_t) 1, pageVar), maxPage);

    doc["count"] = rows;
    doc["pages"] = maxPage;
    doc["page"] = page;

    int16_t start = rows - (page * pageSize);
    uint8_t toGet = min(start, (int16_t) 0) + pageSize;

    LogLine arr[pageSize];
    size_t actual = logger.readRows(arr, max((int16_t) 0, start), toGet);
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