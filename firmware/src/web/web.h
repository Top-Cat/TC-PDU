#include <WebServer.h>
#include <ArduinoJson.h>
#include "auth/jwt.h"
#include "config.h"

class PDUWeb {
  public:
    PDUWeb(WebServer* _server);
    void setup();
    void task();
  private:
    WebServer* server;
    ArduinoJWT jwt;

    void authEndpoints();
    void configEndpoints();
    void updateEndpoints();
    void controlEndpoints();
    void logEndpoints();

    static constexpr const char* textPlain = "text/plain";
    void sendStaticHeaders();
    bool checkCredentials(const char* user, const char* pass);
    bool currentUser(String* const& user, time_t* const& riat = NULL);
    void setSession(JWTConfig* conf, time_t now, const char* user);

    static bool deserializeOrError(WebServer* server, JsonDocument* doc);
    static constexpr const uint8_t pageSize = 16;
};
