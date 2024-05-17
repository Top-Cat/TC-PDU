#ifndef syslog_h
#define syslog_h

#include <WiFiUdp.h>
#include "logs.h"

class SyslogLogger {
  public:
    void process(LogProcess* state);
  private:
    WiFiUDP* udp;
};

extern SyslogLogger syslog;

#endif
