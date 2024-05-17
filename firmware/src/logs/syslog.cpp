#include "syslog.h"
#include "network.h"
#include "config.h"

void SyslogLogger::process(LogProcess* state) {
  while (!(network.setupComplete && network.hasTime())) delay(500);

  SyslogConfig* conf = config.getSyslog();
  if (conf->host.length() <= 0) return;
  if (udp == NULL) udp = new WiFiUDP();

  LogLine* log = state->msg;

  uint8_t priority = 1;
  uint8_t version = 1;
  String hostname = "-";

  char buffer[100];
  time_t time = log->time / 1000;
  struct tm* lt = localtime(&time);
  uint16_t rem = log->time % 1000;

  size_t p = strftime(buffer, sizeof(buffer), "%FT%T", lt);
  p += snprintf(&buffer[p], sizeof(buffer) - p, ".%03d", rem);
  strftime(&buffer[p], sizeof(buffer) - p, "%z", lt);

  udp->beginPacket(conf->host.c_str(), conf->port);
  udp->printf("<%d>%d %s %s tc-pdu - - - [%s] [%s] %s\n", priority, version, buffer, hostname, log->user, LOGTYPE[log->type], log->message);
  udp->endPacket();
}

///// Global object

SyslogLogger syslog;
