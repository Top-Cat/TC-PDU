#include "web.h"
#include <Update.h>
#include "logs/logs.h"

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

void PDUWeb::updateEndpoints() {
  server->on("/", HTTP_GET, [&]() {
    sendStaticHeaders();
    server->send(200, "text/html", serverIndex);
  });

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
