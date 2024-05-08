#include "web.h"

#include "config.h"
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
  logEndpoints();

  server->onNotFound([&]() {
    sendStaticHeaders();
    server->send(200, "text/html", serverIndex);
  });

  const char* headerKeys[] = {"Cookie"};
  size_t headerKeysSize = sizeof(headerKeys) / sizeof(char*);
  server->collectHeaders(headerKeys, headerKeysSize);

  server->begin();
}

void PDUWeb::task() {
  server->handleClient();
}
