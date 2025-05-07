#include <HardwareSerial.h>
#include "logs.h"
#include "filelog.h"
#include "email.h"
#include "syslog.h"
#include "config.h"
#include "network.h"

const char* LOGTYPE[12] = {"Outlet State", "IP", "Firmware", "Crash", "Login", "Login Failure", "Config", "Overload", "Outlet Trip", "Outlet Alarm", "Boot", "Network"};

QueueHandle_t EmailQueue, FileQueue, SyslogQueue;
TaskHandle_t EmailTask, FileTask, SyslogTask;

void handleDelete(LogProcess* state) {
  if (state->emailComplete && state->serialComplete && state->syslogComplete && state->fileComplete) {
    delete state->msg;
    delete state;
  }
}

void PDULogs::printLog(LogProcess* state) {
  LogLine* msg = state->msg;
  time_t time = msg->time / 1000;
  struct tm *ptm = gmtime(&time);

  char buffer[100];
  strftime(buffer, sizeof(buffer), "%F %T", ptm);

  Serial.print("[");
  Serial.print(buffer);
  Serial.print("] [");
  Serial.print(msg->user);
  Serial.print("] [");
  Serial.print(LOGTYPE[msg->type]);
  Serial.print("] ");
  Serial.println(msg->message);

  state->serialComplete = true;
  handleDelete(state);
}

void PDULogs::sendToSyslog(LogProcess* state) {
  xQueueSend(SyslogQueue, &state, 0);
}

void PDULogs::sendEmail(LogProcess* state) {
  xQueueSend(EmailQueue, &state, 0);
}

void PDULogs::toFile(LogProcess* state) {
  xQueueSend(FileQueue, &state, 0);
}

void PDULogs::msg(LogLine* msg) {
  if (msg->time == 0) {
    msg->time = network.getEpochMs();
  }

  LogConfig* logConf = config.getLog();
  LogProcess* logState = new LogProcess();
  logState->msg = msg;
  logState->serialComplete = bitRead(logConf->serialMask, msg->type) != 1;
  logState->syslogComplete = bitRead(logConf->syslogMask, msg->type) != 1;
  logState->emailComplete = bitRead(logConf->emailMask, msg->type) != 1;

  toFile(logState);
  if (!logState->serialComplete) printLog(logState);
  if (!logState->syslogComplete) sendToSyslog(logState);
  if (!logState->emailComplete) sendEmail(logState);
}

size_t PDULogs::getSize() {
  return fileLog.rowCount();
}

size_t PDULogs::getSize(time_t day) {
  return fileLog.rowCount(day);
}

size_t PDULogs::readRows(LogLine* output, size_t skip, size_t maxCount) {
  return fileLog.readRows(output, skip, maxCount);
}

size_t PDULogs::readRows(LogLine* output, time_t day, size_t skip, size_t maxCount) {
  return fileLog.readRows(output, day, skip, maxCount);
}

void PDULogs::format() {
  fileLog.format();
}

///// Global object

PDULogs logger;

///// Task

void emailTask(void * parameter) {
  LogProcess* state;

  while (1) {
    if (xQueueReceive(EmailQueue, &(state), 10) != pdPASS) continue;

    emailLog.process(state);

    state->emailComplete = true;
    handleDelete(state);
  }
}

///// File Task

void fileTask(void * parameter) {
  LogProcess* state;

  while (1) {
    if (xQueueReceive(FileQueue, &(state), 10) != pdPASS) continue;

    fileLog.process(state);

    state->fileComplete = true;
    handleDelete(state);
  }
}

///// Syslog Task

void syslogTask(void * parameter) {
  LogProcess* state;

  while (1) {
    if (xQueueReceive(SyslogQueue, &(state), 10) != pdPASS) continue;

    syslog.process(state);

    state->syslogComplete = true;
    handleDelete(state);
  }
}

void PDULogs::setupTask() {
  SPIFFS.begin(true);

  LogProcess* test;
  EmailQueue = xQueueCreate(5, sizeof(&test));
  FileQueue = xQueueCreate(5, sizeof(&test));
  SyslogQueue = xQueueCreate(5, sizeof(&test));
  xTaskCreatePinnedToCore(emailTask, "Log2Email", 8192, NULL, 0, &EmailTask, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(fileTask, "Log2File", 8192, NULL, 0, &FileTask, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(syslogTask, "Log2Syslog", 8192, NULL, 0, &SyslogTask, tskNO_AFFINITY);
}
