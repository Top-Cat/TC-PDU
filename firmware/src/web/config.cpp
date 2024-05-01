#include "web.h"
#include "config.h"

void PDUWeb::configEndpoints() {
  server->on("/config", HTTP_GET, [&]() {
    JsonDocument doc;
    WifiConfig* wifiConf = config.getWifi();
    doc["wifi"]["ssid"] = wifiConf->ssid;
    doc["wifi"]["pass"] = wifiConf->password;

    RadiusConfig* radiusConf = config.getRadius();
    doc["radius"]["ip"] = radiusConf->ip.toString();
    doc["radius"]["port"] = radiusConf->port;
    doc["radius"]["secret"] = radiusConf->secret;
    doc["radius"]["timeout"] = radiusConf->timeout;
    doc["radius"]["retries"] = radiusConf->retries;

    JWTConfig* jwtConf = config.getJWT();
    doc["auth"]["validityPeriod"] = jwtConf->validityPeriod;
    //doc["auth"]["key"] = jwtConf->key;
    //doc["auth"]["adminPassword"] = config.adminPassword;

    NTPConfig* ntpConf = config.getNTP();
    doc["ntp"]["host"] = ntpConf->host;
    doc["ntp"]["offset"] = ntpConf->offset;

    LogConfig* logConf = config.getLog();
    doc["log"]["serialMask"] = logConf->serialMask;
    doc["log"]["syslogMask"] = logConf->syslogMask;
    doc["log"]["emailMask"] = logConf->emailMask;

    doc["log"]["smtp"]["host"] = logConf->smtpServer;
    doc["log"]["smtp"]["port"] = logConf->smtpPort;
    doc["log"]["smtp"]["user"] = logConf->smtpUser;
    doc["log"]["smtp"]["password"] = logConf->smtpPass;
    doc["log"]["smtp"]["from"] = logConf->smtpFrom;
    doc["log"]["smtp"]["to"] = logConf->smtpTo;

    String json;
    serializeJson(doc, json);

    sendStaticHeaders();
    server->send(200, "application/json", json);
  });

  server->on("/config/wifi", HTTP_POST, [&]() {
    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    WifiConfig* wifiConf = config.getWifi();
    wifiConf->ssid = (const char*) doc["ssid"];
    wifiConf->password = (const char*) doc["pass"];

    config.save();

    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, []() { });

  server->on("/config/radius", HTTP_POST, [&]() {
    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    RadiusConfig* radiusConf = config.getRadius();
    if (doc["secret"]) radiusConf->secret = (const char*) doc["secret"];
    if (doc["port"]) radiusConf->port = doc["port"];
    if (doc["ip"]) radiusConf->ip.fromString((const char*) doc["ip"]);

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/config/ntp", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    NTPConfig* ntpConf = config.getNTP();
    if (doc["host"]) ntpConf->host = (const char*) doc["host"];
    if (doc["offset"]) ntpConf->offset = doc["offset"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/config/auth", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    JWTConfig* jwtConf = config.getJWT();
    if (doc["validityPeriod"]) jwtConf->validityPeriod = doc["validityPeriod"];
    if (doc["updateKey"]) config.regenerateJWTKey();
    if (user == "admin" && doc["adminPassword"]) config.adminPassword = (const char*) doc["adminPassword"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/config/log", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogConfig* logConf = config.getLog();
    if (doc["serialMask"]) logConf->serialMask = doc["serialMask"];
    if (doc["emailMask"]) logConf->emailMask = doc["emailMask"];
    if (doc["syslogMask"]) logConf->syslogMask = doc["syslogMask"];

    if (doc["smtp"]) {
      if (doc["smtp"]["host"]) logConf->smtpServer = (const char*) doc["smtp"]["host"];
      if (doc["smtp"]["port"]) logConf->smtpPort = doc["smtp"]["port"];
      if (doc["smtp"]["user"]) logConf->smtpUser = (const char*) doc["smtp"]["user"];
      if (doc["smtp"]["password"]) logConf->smtpPass = (const char*) doc["smtp"]["password"];
      if (doc["smtp"]["from"]) logConf->smtpFrom = (const char*) doc["smtp"]["from"];
      if (doc["smtp"]["to"]) logConf->smtpTo = (const char*) doc["smtp"]["to"];
    }

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });
}