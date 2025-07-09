#include "web.h"
#include "config.h"
#include "../logs/logs.h"
#include "mqtt.h"
#include "network.h"

void PDUWeb::configEndpoints() {
  server->on("/api/config", HTTP_GET, [&]() {
    String user;
    if (!currentUser(&user)) return;

    JsonDocument doc;
    WifiConfig* wifiConf = config.getWifi();
    doc["wifi"]["enabled"] = wifiConf->enabled;
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
    doc["auth"]["adminPassword"] = config.adminPassword;

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
    doc["mqtt"]["enabled"] = mqttConf->enabled;
    doc["mqtt"]["host"] = mqttConf->host;
    doc["mqtt"]["port"] = mqttConf->port;
    doc["mqtt"]["user"] = mqttConf->username;
    doc["mqtt"]["password"] = mqttConf->password;
    doc["mqtt"]["clientId"] = mqttConf->clientId;
    doc["mqtt"]["prefix"] = mqttConf->prefix;
    doc["mqtt"]["addMacToPrefix"] = mqttConf->addMacToPrefix;

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
    if (!currentUser(&user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "WiFi config updated", sizeof(msg->message));
    logger.msg(msg);

    WifiConfig* wifiConf = config.getWifi();

    JsonVariant enabled = doc["enabled"];
    if (!enabled.isNull()) wifiConf->enabled = enabled;

    wifiConf->ssid = doc["ssid"].as<const char*>();
    wifiConf->password = doc["pass"].as<const char*>();

    config.save();

    network.reconfigureWifi();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, []() { });

  server->on("/api/config/radius", HTTP_POST, [&]() {
    String user;
    if (!currentUser(&user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "Radius config updated", sizeof(msg->message));
    logger.msg(msg);

    RadiusConfig* radiusConf = config.getRadius();
    if (doc["secret"]) radiusConf->secret = doc["secret"].as<const char*>();
    if (doc["port"]) radiusConf->port = doc["port"];
    if (doc["ip"]) radiusConf->ip.fromString(doc["ip"].as<const char*>());

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/ntp", HTTP_POST, [&]() {
    String user;
    if (!currentUser(&user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "NTP config updated", sizeof(msg->message));
    logger.msg(msg);

    NTPConfig* ntpConf = config.getNTP();
    if (doc["host"]) ntpConf->host = doc["host"].as<const char*>();
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
    time_t iat;
    if (!currentUser(&user, &iat)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "Auth config updated", sizeof(msg->message));
    logger.msg(msg);

    JWTConfig* jwtConf = config.getJWT();
    if (doc["validityPeriod"]) jwtConf->validityPeriod = doc["validityPeriod"];
    if (doc["updateKey"]) {
      JWTConfig* jwtConf = config.getJWT();
      config.regenerateJWTKey();
      jwt.setPSK(jwtConf->key);
      // Use issed at time as now to re-issue same validity as before
      setSession(jwtConf, iat, user.c_str());
    }

    if (doc["adminPassword"] && config.adminPassword.equals(doc["oldPassword"].as<const char*>())) {
      config.adminPassword = doc["adminPassword"].as<const char*>();
    }

    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
    config.save();
  }, [&]() { });

  server->on("/api/config/log", HTTP_POST, [&]() {
    String user;
    if (!currentUser(&user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "Log config updated", sizeof(msg->message));
    logger.msg(msg);

    LogConfig* logConf = config.getLog();
    if (doc["serialMask"].is<uint64_t>()) logConf->serialMask = doc["serialMask"];
    if (doc["emailMask"].is<uint64_t>()) logConf->emailMask = doc["emailMask"];
    if (doc["syslogMask"].is<uint64_t>()) logConf->syslogMask = doc["syslogMask"];
    if (doc["days"].is<uint8_t>()) logConf->daysToKeep = doc["days"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/smtp", HTTP_POST, [&]() {
    String user;
    if (!currentUser(&user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "SMTP config updated", sizeof(msg->message));
    logger.msg(msg);

    LogConfig* logConf = config.getLog();
    if (doc["host"]) logConf->smtpServer = doc["host"].as<const char*>();
    if (doc["port"]) logConf->smtpPort = doc["port"];
    if (doc["user"]) logConf->smtpUser = doc["user"].as<const char*>();
    if (doc["password"]) logConf->smtpPass = doc["password"].as<const char*>();
    if (doc["from"]) logConf->smtpFrom = doc["from"].as<const char*>();
    if (doc["to"]) logConf->smtpTo = doc["to"].as<const char*>();

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/mqtt", HTTP_POST, [&]() {
    String user;
    if (!currentUser(&user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "MQTT config updated", sizeof(msg->message));
    logger.msg(msg);

    MqttConfig* mqttConf = config.getMqtt();
    JsonVariant enabled = doc["enabled"];
    if (!enabled.isNull()) mqttConf->enabled = enabled;
    if (doc["host"]) mqttConf->host = doc["host"].as<const char*>();
    if (doc["port"]) mqttConf->port = doc["port"];
    if (doc["user"]) mqttConf->username = doc["user"].as<const char*>();
    if (doc["password"]) mqttConf->password = doc["password"].as<const char*>();
    if (doc["clientId"]) mqttConf->clientId = doc["clientId"].as<const char*>();
    if (doc["prefix"]) mqttConf->prefix = doc["prefix"].as<const char*>();
    if (doc["addMacToPrefix"].is<bool>()) mqttConf->addMacToPrefix = (bool) doc["addMacToPrefix"];

    config.save();
    mqtt.triggerChanges();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });

  server->on("/api/config/syslog", HTTP_POST, [&]() {
    String user;
    if (!currentUser(&user)) return;

    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    LogLine* msg = new LogLine();
    msg->type = CONFIG;
    strncpy(msg->user, user.c_str(), sizeof(msg->user));
    strncpy(msg-> message, "Syslog config updated", sizeof(msg->message));
    logger.msg(msg);

    SyslogConfig* slogConf = config.getSyslog();
    if (doc["host"]) slogConf->host = doc["host"].as<const char*>();
    if (doc["port"]) slogConf->port = doc["port"];

    config.save();
    sendStaticHeaders();
    server->send(200, textPlain, "DONE");
  }, [&]() { });
}
