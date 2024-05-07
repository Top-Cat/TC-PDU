#ifndef logs_h
#define logs_h

#include <ESP_Mail_Client.h>
#include <time.h>

enum LogType { OUTLET_STATE, DEVICE_IP, FIRMWARE, CRASH, LOGIN_SUCCESS, LOGIN_FAILURE, CONFIG };
extern const char* LOGTYPE[7];

struct LogLine {
  time_t time = 0;
  LogType type;
  char user[64] = "SYSTEM";
  char message[128];
};

struct LogProcess {
  LogLine* msg;
  bool serialComplete;
  bool syslogComplete;
  bool emailComplete;
  bool fileComplete;
};

class PDULogs {
  public:
    void msg(LogLine* msg);
    void setupTask();

    size_t getSize();
    size_t readRows(LogLine* output, size_t startIdx, size_t maxCount);
  private:
    void toFile(LogProcess* msg);
    void printLog(LogProcess* state);
    void sendToSyslog(LogProcess* state);
    void sendEmail(LogProcess* state);
};

extern PDULogs logger;

#endif
