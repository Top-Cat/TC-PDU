#include "control.h"

#include "i2c.h"
#include "config.h"
#include "network.h"
#include "logs/logs.h"

Output* PDUControl::getLowestPriority() {
  uint8_t priority = 0xFF;
  Output* res = NULL;
  for (uint8_t idx = 0; idx < MAX_OUTPUTS; idx++) {
    Output* out = config.getOutput(idx);
    if (out->getPriority() < priority && out->getState()) {
      res = out;
      priority = out->getPriority();
    }
  }

  return res;
}

uint32_t PDUControl::getUptime() {
  uint64_t uptime = esp_timer_get_time();
  return uptime / 1000000;
}

void PDUControl::task() {
  while (!bus.initComplete) delay(1000);

  for (uint8_t idx = 0; idx < MAX_OUTPUTS; idx++) {
    config.getOutput(idx)->init();
  }

  uint8_t mod = 0;
  while (true) {
    float localTotalPower = 0;
    uint64_t time = esp_timer_get_time();
    for (uint8_t idx = 0; idx < MAX_OUTPUTS; idx++) {
      Output* out = config.getOutput(idx);
      out->tick(time);

      if (out->getState()) {
        localTotalPower += out->getPower();
      }
    }

    // Turn off lowest priority output if overloaded
    if (localTotalPower > 240 * 10) {
      Output* res = getLowestPriority();

      if (res != NULL) {
        LogLine* msg = new LogLine();
        msg->type = OVERLOAD;
        snprintf(msg->message, sizeof(msg->message), "Turning off output due to overload condition. Total: %.1f W", localTotalPower);
        logger.msg(msg);

        res->setState(NULL, false);
      }
    }

    totalPower = localTotalPower;
    delay(500);
  }
}

float PDUControl::getTotalPower() {
  return totalPower;
}

///// Global object

PDUControl control;

///// Task

TaskHandle_t ControlTask;

void controlTask(void * parameter) {
  control.task();
}

void PDUControl::setupTask() {
  xTaskCreatePinnedToCore(controlTask, "ControlTask", 4096, NULL, 0, &ControlTask, 0);
}
