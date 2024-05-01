#ifndef ARDUINO_JWT_H
#define ARDUINO_JWT_H

#include <WString.h>

class ArduinoJWT {
  private:
    String _psk;

  public:
    ArduinoJWT() : ArduinoJWT(F("")) {};
    ArduinoJWT(String psk);
    ArduinoJWT(char* psk);

    // Set a new psk for encoding and decoding JWTs
    void setPSK(String psk);
    void setPSK(char* psk);

    // Get the calculated length of a JWT
    int getJWTLength(String& payload);
    int getJWTLength(char* payload);
    // Get the length of the decoded payload from a JWT
    int getJWTPayloadLength(String& jwt);
    int getJWTPayloadLength(char* jwt);
    // Create a JSON Web Token
    String encodeJWT(String& payload);
    void encodeJWT(char* payload, char* jwt);
    // Decode a JWT and retreive the payload
    bool decodeJWT(String& jwt, String& payload);
    bool decodeJWT(char* jwt, char* payload, int payloadLength);
};

#endif
