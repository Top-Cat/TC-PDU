#ifndef logs_h
#define logs_h

#include <time.h>

enum LogType { OUTLET_STATE, DEVICE_IP, FIRMWARE, CRASH, LOGIN_SUCCESS, LOGIN_FAILURE, CONFIG, OVERLOAD, TRIP, ALARM, BOOT, NETWORK };
extern const char* LOGTYPE[12];

struct LogLine {
  uint64_t time = 0;
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

    void format();

    size_t getSize();
    size_t readRows(LogLine* output, size_t startIdx, size_t maxCount);

    size_t getSize(time_t day);
    size_t readRows(LogLine* output, time_t day, size_t startIdx, size_t maxCount);
  private:
    void toFile(LogProcess* msg);
    void printLog(LogProcess* state);
    void sendToSyslog(LogProcess* state);
    void sendEmail(LogProcess* state);
};

extern PDULogs logger;

#endif
