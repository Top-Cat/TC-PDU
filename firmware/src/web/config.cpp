#include "web.h"
#include "config.h"
#include "../logs/logs.h"

void PDUWeb::configEndpoints() {
  server->on("/api/config", HTTP_GET, [&]() {
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
    doc["ntp"]["tz"] = ntpConf->timezone;

    LogConfig* logConf = config.getLog();
    doc["log"]["serialMask"] = logConf->serialMask;
    doc["log"]["syslogMask"] = logConf->syslogMask;
    doc["log"]["emailMask"] = logConf->emailMask;
    doc["log"]["days"] = logConf->daysToKeep;

    doc["log"]["smtp"]["host"] = logConf->smtpServer;
    doc["log"]["smtp"]["port"] = logConf->smtpPort;
    doc["log"]["smtp"]["user"] = logConf->smtpUser;
    doc["log"]["smtp"]["password"] = logConf->smtpPass;
    doc["log"]["smtp"]["from"] = logConf->smtpFrom;
    doc["log"]["smtp"]["to"] = logConf->smtpTo;

    MqttConfig* mqttConf = config.getMqtt();
    doc["mqtt"]["host"] = mqttConf->host;
    doc["mqtt"]["port"] = mqttConf->port;
    doc["mqtt"]["user"] = mqttConf->username;
    doc["mqtt"]["password"] = mqttConf->password;
    doc["mqtt"]["clientId"] = mqttConf->clientId;
    doc["mqtt"]["prefix"] = mqttConf->prefix;

    SyslogConfig* slogConf = config.getSyslog();
    doc["syslog"]["host"] = slogConf->host;
    doc["syslog"]["port"] = slogConf->port;

    String json;
    serializeJson(doc, json);

    sendStaticHeaders();
    server->send(200, "application/json", json);
  });

  server->on("/api/config/wifi", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "WiFi config updated", sizeof(msg->message));
    logger.msg(msg);

    WifiConfig* wifiConf = config.getWifi();
    wifiConf->ssid = (const char*) doc["ssid"];
    wifiConf->password = (const char*) doc["pass"];

    config.save();

    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, []() { });

  server->on("/api/config/radius", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "Radius config updated", sizeof(msg->message));
    logger.msg(msg);

    RadiusConfig* radiusConf = config.getRadius();
    if (doc["secret"]) radiusConf->secret = (const char*) doc["secret"];
    if (doc["port"]) radiusConf->port = doc["port"];
    if (doc["ip"]) radiusConf->ip.fromString((const char*) doc["ip"]);

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/ntp", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "NTP config updated", sizeof(msg->message));
    logger.msg(msg);

    NTPConfig* ntpConf = config.getNTP();
    if (doc["host"]) ntpConf->host = (const char*) doc["host"];
    if (doc["tz"]) {
      const char* tz = doc["tz"];
      ntpConf->timezone = tz;
      setenv("TZ", tz, 1);
      tzset();
    }

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/auth", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "Auth config updated", sizeof(msg->message));
    logger.msg(msg);

    JWTConfig* jwtConf = config.getJWT();
    if (doc["validityPeriod"]) jwtConf->validityPeriod = doc["validityPeriod"];
    if (doc["updateKey"]) config.regenerateJWTKey();
    if (user == "admin" && doc["adminPassword"]) config.adminPassword = (const char*) doc["adminPassword"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/log", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "Log config updated", sizeof(msg->message));
    logger.msg(msg);

    LogConfig* logConf = config.getLog();
    if (doc["serialMask"]) logConf->serialMask = doc["serialMask"];
    if (doc["emailMask"]) logConf->emailMask = doc["emailMask"];
    if (doc["syslogMask"]) logConf->syslogMask = doc["syslogMask"];
    if (doc["days"]) logConf->daysToKeep = doc["days"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/smtp", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "SMTP config updated", sizeof(msg->message));
    logger.msg(msg);

    LogConfig* logConf = config.getLog();
    if (doc["host"]) logConf->smtpServer = (const char*) doc["host"];
    if (doc["port"]) logConf->smtpPort = doc["port"];
    if (doc["user"]) logConf->smtpUser = (const char*) doc["user"];
    if (doc["password"]) logConf->smtpPass = (const char*) doc["password"];
    if (doc["from"]) logConf->smtpFrom = (const char*) doc["from"];
    if (doc["to"]) logConf->smtpTo = (const char*) doc["to"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/mqtt", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "MQTT config updated", sizeof(msg->message));
    logger.msg(msg);

    MqttConfig* mqttConf = config.getMqtt();
    if (doc["host"]) mqttConf->host = (const char*) doc["host"];
    if (doc["port"]) mqttConf->port = doc["port"];
    if (doc["user"]) mqttConf->username = (const char*) doc["user"];
    if (doc["password"]) mqttConf->password = (const char*) doc["password"];
    if (doc["clientId"]) mqttConf->clientId = (const char*) doc["clientId"];
    if (doc["prefix"]) mqttConf->prefix = (const char*) doc["prefix"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/syslog", HTTP_POST, [&]() {
    String user;
    if (!currentUser(user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "Syslog config updated", sizeof(msg->message));
    logger.msg(msg);

    SyslogConfig* slogConf = config.getSyslog();
    if (doc["host"]) slogConf->host = (const char*) doc["host"];
    if (doc["port"]) slogConf->port = doc["port"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });
}
