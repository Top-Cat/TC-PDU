#include <esp_task_wdt.h>

#include "network.h"
#include "config.h"
#include "logs/logs.h"
#include "i2c.h"
#include "web/web.h"
#include "control.h"
#include "mqtt.h"

#define WDT_TIMEOUT 3

WebServer server(80);
PDUWeb web = PDUWeb(&server);

void logStartupError(const char* reason) {
  LogLine* msg = new LogLine();
  msg->type = CRASH;
  snprintf(msg->message, sizeof(msg->message), "System reset due to %s", reason);
  logger.msg(msg);
}

void logStartupError() {
  switch (esp_reset_reason()) {
    case ESP_RST_BROWNOUT:
      logStartupError("brownout");
      break;
    case ESP_RST_PANIC:
      logStartupError("exception");
      break;
    case ESP_RST_INT_WDT:
      logStartupError("interrupt watchdog");
      break;
    case ESP_RST_TASK_WDT:
      logStartupError("task watchdog");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.print(F("setup() running on core "));
  Serial.println(xPortGetCoreID());

  config.load();

  logger.setupTask();
  bus.setupTask();
  network.setupTask();
  control.setupTask();
  mqtt.setupTask();

  logStartupError();

  // Setup watchdog
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  while (!network.setupComplete) {
    esp_task_wdt_reset();
    delay(100);
  }

  web.setup();
}

void loop() {
  web.task();
  esp_task_wdt_reset();
  delay(1);
}