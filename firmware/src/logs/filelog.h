#ifndef filelog_h
#define filelog_h

#include <FS.h>
#include <SPIFFS.h>
#include "logs.h"

class FileLogger {
  public:
    void process(LogProcess* msg);
    size_t readRows(LogLine* output, time_t date, size_t startIdx, size_t maxCount);
    size_t rowCount(time_t time);
  private:
    void rotate();
    size_t rowCount(File &logFile);

    time_t today;
    char curPath[32] = "";

    static void pathFromDate(char* output, size_t size, time_t time);
    static time_t dateFromPath(const char* path);

    static constexpr const char* directory = "/logs";
    static constexpr const char* pathFormat = "%s/%s.log";
    static constexpr const uint8_t daysToKeep = 1;
};

extern FileLogger fileLog;

#endif
