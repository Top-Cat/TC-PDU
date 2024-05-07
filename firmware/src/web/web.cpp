#include "web.h"
#include <uri/UriBraces.h>

#include "config.h"
#include "logs/logs.h"
#include "version.h"

const char* serverIndex = "<!DOCTYPE HTML><html lang=\"en\"><head><title>TC-PDU</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><link href=\"https://use.fontawesome.com/releases/v5.15.4/css/all.css\" rel=\"stylesheet\"><link href=\"https://pdu.topc.at/" STRING(ASSET_VERSION) "/main.css\" rel=\"stylesheet\"><script src=\"https://pdu.topc.at/" STRING(ASSET_VERSION) "/ext.js\"></script><script src=\"https://pdu.topc.at/" STRING(ASSET_VERSION) "/output.js\"></script></head><body><main class=\"container\" id=\"root\"></main></body></html>";

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

  server->onNotFound([&]() {
    sendStaticHeaders();
    server->send(200, "text/html", serverIndex);
  });

  server->on(UriBraces("/api/logs/{}"), HTTP_GET, [&]() {

    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    JsonArray logs = doc["logs"].to<JsonArray>();

    size_t rows = logger.getSize();
    uint8_t pageVar = strtoul(server->pathArg(0).c_str(), NULL, 0);
    uint8_t maxPage = ceil(rows / (float) pageSize);
    uint8_t page = min(max((uint8_t) 1, pageVar), maxPage);

    doc["count"] = rows;
    doc["pages"] = maxPage;
    doc["page"] = page;

    LogLine arr[pageSize];
    size_t skip = (page - 1) * pageSize;
    size_t actual = logger.readRows(arr, skip, pageSize);

    for (uint8_t idx = 1; idx <= actual; idx++) {
      LogLine* line = &arr[pageSize - idx];

      JsonObject object = logs.add<JsonObject>();
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