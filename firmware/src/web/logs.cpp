#include "web.h"
#include <uri/UriBraces.h>

#include "config.h"
#include "logs/logs.h"
#include "network.h"

void PDUWeb::logEndpoints() {
  server->on(UriBraces("/api/logs/{}"), HTTP_GET, [&]() {

    String user;
    if (!currentUser(&user)) return;

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

  server->on(UriBraces("/api/logf/{}/{}"), HTTP_GET, [&]() {

    String user;
    if (!currentUser(&user)) return;

    JsonDocument doc;
    JsonArray logs = doc["logs"].to<JsonArray>();

    uint8_t dayVar = strtoul(server->pathArg(0).c_str(), NULL, 0);
    uint8_t pageVar = strtoul(server->pathArg(1).c_str(), NULL, 0);

    time_t now = network.getEpochTime();
    time_t day = now - (dayVar * 86400);

    size_t rows = logger.getSize(day);
    uint8_t maxPage = ceil(rows / (float) pageSize);
    uint8_t page = min(max((uint8_t) 1, pageVar), maxPage);

    doc["count"] = rows;
    doc["pages"] = maxPage;
    doc["page"] = page;

    LogLine arr[pageSize];
    size_t skip = (page - 1) * pageSize;
    size_t actual = logger.readRows(arr, day, skip, pageSize);

    for (uint8_t idx = 0; idx < actual; idx++) {
      LogLine* line = &arr[idx];

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

  server->on("/api/format", HTTP_POST, [&]() {
    String user;
    if (!currentUser(&user)) return;

    logger.format();

    sendStaticHeaders();
    server->send(200, textPlain, "Done");
  }, [&]() { });
}
