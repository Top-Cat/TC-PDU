#include "web.h"
#include <Update.h>
#include "logs/logs.h"

#ifndef BUILD_NUMBER
#warning "Build number not set"
#define BUILD_NUMBER 8
#endif
#define STR_HELPER(x) #x
#define STRING(x) STR_HELPER(x)

const char* serverIndex = "<!DOCTYPE HTML><html lang=\"en\"><head><title>TC-PDU</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><link href=\"https://use.fontawesome.com/releases/v5.15.4/css/all.css\" rel=\"stylesheet\"><link href=\"https://pdu.topc.at/" STRING(BUILD_NUMBER) "/main.css\" rel=\"stylesheet\"><script src=\"https://pdu.topc.at/" STRING(BUILD_NUMBER) "/ext.js\"></script><script src=\"https://pdu.topc.at/" STRING(BUILD_NUMBER) "/output.js\"></script></head><body><main class=\"container\" id=\"root\"></main></body></html>";

void PDUWeb::updateEndpoints() {
  server->on("/", HTTP_GET, [&]() {
    sendStaticHeaders();
    server->send(200, "text/html", serverIndex);
  });

  // TODO: Also respond with basic html to other endpoints

  server->on("/update", HTTP_POST, [&]() {
    sendStaticHeaders();
    server->send(200, textPlain, (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, [&]() {
    HTTPUpload& upload = server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      LogLine* msg = new LogLine();
      msg->type = FIRMWARE;
      snprintf(msg->message, sizeof(msg->message), "Firmware upgrade started: %s", upload.filename.c_str());
      logger.msg(msg);

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        LogLine* msg = new LogLine();
        msg->type = FIRMWARE;
        strcpy(msg->message, "Firmware upgrade success");
        logger.msg(msg);
      } else {
        Update.printError(Serial);
      }
    }
  });
}
