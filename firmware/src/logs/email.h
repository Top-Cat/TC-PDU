#ifndef emaillog_h
#define emaillog_h

#include <ESP_Mail_Client.h>
#include <time.h>
#include "logs.h"
#include "config.h"

class EmailLogger {
  public:
    void process(LogProcess* state);
  private:
    SMTPSession smtp;
    Session_Config setupConfig(LogConfig* logConf);
};

extern EmailLogger emailLog;

#endif
