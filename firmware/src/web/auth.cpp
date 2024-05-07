#include "web.h"
#include "../auth/radius.h"
#include "config.h"
#include "network.h"
#include "../logs/logs.h"

bool PDUWeb::checkCredentials(const char* user, const char* pass) {
  // Local auth
  if (strcmp(user, "admin") == 0 && config.adminPassword.equals(pass)) return true;

  // Radius auth
  RadiusConfig* radiusConf = config.getRadius();
  const char* secret = radiusConf->secret.c_str();
  uint16_t secretLen = radiusConf->secret.length();

  RadiusMsg msg(RadiusCodeAccessRequest, radiusConf->timeout, radiusConf->retries);
  msg.addAttr(RadiusAttrUserName, 0, user);
  msg.addAttr(RadiusAttrUserPassword, 0, pass);
  msg.sign(secret, secretLen);

  RadiusMsg reply;

  return msg.sendWaitReply(&udpClient, radiusConf->ip, radiusConf->port, &reply) &&
    reply.checkAuthenticatorsWithOriginal(secret, secretLen, &msg) &&
    reply.code() == RadiusCodeAccessAccept;
}

bool PDUWeb::currentUser(String& user) {
  String cookieStr = server->header("Cookie");
  int delimIndex;

  do {
    if (cookieStr.length() == 0) break;

    delimIndex = cookieStr.indexOf(";");

    String cookie;
    if (delimIndex >= 0) {
      cookie = cookieStr.substring(0, delimIndex);
      cookieStr = cookieStr.substring(delimIndex + 1);
    } else {
      cookie = cookieStr;
    }

    int eqIndex = cookie.indexOf("=");
    String name = cookie.substring(0, eqIndex);
    String val = cookie.substring(eqIndex + 1);

    if (name == "PDUJWT") {
      String payload;
      if (jwt.decodeJWT(val, payload)) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error && doc["exp"] > network.getEpochTime() && doc["iat"] < network.getEpochTime()) {
          String sub = doc["sub"];
          user = sub;
          return true;
        }
      }
    }
  } while (delimIndex >= 0);

  sendStaticHeaders();
  server->send(401, "text/html", "Forbidden");
  return false;
}

void PDUWeb::authEndpoints() {
  server->on("/api/me", HTTP_GET, [&]() {
    String user;
    if (!currentUser(user)) return;

    sendStaticHeaders();
    server->send(200, "text/html", user);
  });

  server->on("/api/login", HTTP_POST, [&]() {
    JsonDocument doc;
    if (!deserializeOrError(server, &doc)) return;

    const char* user = doc["user"];
    const char* pass = doc["pass"];

    LogLine* msg = new LogLine();
    strncpy(msg->user, user, 64);

    sendStaticHeaders();
    if (checkCredentials(user, pass)) {
      msg->type = LOGIN_SUCCESS;
      JWTConfig* jwtConf = config.getJWT();

      JsonDocument payload;
      payload["sub"] = user;
      payload["iat"] = network.getEpochTime();
      payload["exp"] = network.getEpochTime() + jwtConf->validityPeriod;

      String json;
      serializeJson(payload, json);

      server->sendHeader("Location", "/me");
      server->sendHeader("Set-Cookie", "PDUJWT=" + jwt.encodeJWT(json) + "; Path=/");
      server->send(302);
    } else {
      msg->type = LOGIN_FAILURE;
      server->send(401, textPlain, "Forbidden");
    }

    logger.msg(msg);
  }, [&]() { });
}