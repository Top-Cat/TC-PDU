#include "web.h"
#include <Update.h>
#include "logs/logs.h"

void PDUWeb::updateEndpoints() {
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
