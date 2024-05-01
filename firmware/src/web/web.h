#include <WebServer.h>
#include <ArduinoJson.h>
#include "auth/jwt.h"

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

    static constexpr const char* textPlain = "text/plain";
    void sendStaticHeaders();
    bool checkCredentials(const char* user, const char* pass);
    bool currentUser(String& user);

    static bool deserializeOrError(WebServer* server, JsonDocument* doc);
};
