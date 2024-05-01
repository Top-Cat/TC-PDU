#include "jwt.h"
#include "../crypto/base64.h"
#include "../crypto/sha256.h"

// The standard JWT header already base64 encoded. Equates to {"alg": "HS256", "typ": "JWT"}
const PROGMEM char* jwtHeader = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";

ArduinoJWT::ArduinoJWT(String psk) {
  _psk = psk;
}

ArduinoJWT::ArduinoJWT(char* psk) {
  _psk = String(psk);
}

void ArduinoJWT::setPSK(String psk) {
  _psk = psk;
}
void ArduinoJWT::setPSK(char* psk) {
  _psk = String(psk);
}

int ArduinoJWT::getJWTLength(String& payload) {
  return getJWTLength((char*)payload.c_str());
}

int ArduinoJWT::getJWTLength(char* payload) {
  return strlen(jwtHeader) + encode_base64_length(strlen(payload)) + encode_base64_length(32) + 2;
}

int ArduinoJWT::getJWTPayloadLength(String& jwt) {
  return getJWTPayloadLength((char*)jwt.c_str());
}

int ArduinoJWT::getJWTPayloadLength(char* jwt) {
  char jwtCopy[strlen(jwt)];
  memcpy((char*)jwtCopy, jwt, strlen(jwt));
    // Get all three jwt parts
  const char* sep = ".";
  char* token;
  token = strtok(jwtCopy, sep);
  token = strtok(NULL, sep);
  if(token == NULL) {
    return -1;
  } else {
    return decode_base64_length((unsigned char*)token) + 1;
  }
}

String ArduinoJWT::encodeJWT(String& payload) {
  char jwt[getJWTLength(payload)];
  encodeJWT((char*)payload.c_str(), (char*)jwt);
  return String(jwt);
}

void ArduinoJWT::encodeJWT(char* payload, char* jwt) {
  unsigned char* ptr = (unsigned char*)jwt;
  // Build the initial part of the jwt (header.payload)
  memcpy(ptr, jwtHeader, strlen(jwtHeader));
  ptr += strlen(jwtHeader);
  *ptr++ = '.';
  encode_base64((unsigned char*)payload, strlen(payload), ptr);
  ptr += encode_base64_length(strlen(payload));
  // Get rid of any padding (trailing '=' added when base64 encoding)
  while(*(ptr - 1) == '=') {
    ptr--;
  }
  *(ptr) = 0;
  // Build the signature
  Sha256.initHmac((const unsigned char*)_psk.c_str(), _psk.length());
  Sha256.print(jwt);
  // Add the signature to the jwt
  *ptr++ = '.';
  encode_base64(Sha256.resultHmac(), 32, ptr);
  ptr += encode_base64_length(32);
  // Get rid of any padding and replace / and +
  while(*(ptr - 1) == '=') {
    ptr--;
  }
  *(ptr) = 0;
}

bool ArduinoJWT::decodeJWT(String& jwt, String& payload) {
  int payloadLength = getJWTPayloadLength(jwt);
  if(payloadLength > 0) {
    char jsonPayload[payloadLength];
    if(decodeJWT((char*)jwt.c_str(), (char*)jsonPayload, payloadLength)) {
      payload = String(jsonPayload);
      return true;
    }
  }
  return false;
}

bool ArduinoJWT::decodeJWT(char* jwt, char* payload, int payloadLength) {
  // Get all three jwt parts
  const char* sep = ".";
  char* encodedHeader = strtok(jwt, sep);
  char* encodedPayload = strtok(NULL, sep);
  char* encodedSignature = strtok(NULL, sep);

  // Check all three jwt parts exist
  if(encodedHeader == NULL || encodedPayload == NULL || encodedSignature == NULL)
  {
    payload = NULL;
    return false;
  }

  // Build the signature
  Sha256.initHmac((const unsigned char*)_psk.c_str(), _psk.length());
  Sha256.print(encodedHeader);
  Sha256.print(".");
  Sha256.print(encodedPayload);

  // Encode the signature as base64
  unsigned char base64Signature[encode_base64_length(32)];
  encode_base64(Sha256.resultHmac(), 32, base64Signature);
  unsigned char* ptr = &base64Signature[0] + encode_base64_length(32);
  // Get rid of any padding and replace / and +
  while(*(ptr - 1) == '=') {
    ptr--;
  }
  *(ptr) = 0;

  // Do the signatures match?
  if(strcmp((char*)encodedSignature, (char*)base64Signature) == 0) {
    // Decode the payload
    decode_base64((unsigned char*)encodedPayload, (unsigned char*)payload);
    payload[payloadLength - 1] = 0;
    return true;
  } else {
    payload = NULL;
    return false;
  }
}
