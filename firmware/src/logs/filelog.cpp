#include "filelog.h"
#include "network.h"

void FileLogger::process(LogProcess* state) {
  while (!network.hasTime()) delay(500);

  LogLine* msg = state->msg;

  if (msg->time < 10000) {
    msg->time += network.getEpochTime() - (millis() / 1000);
  }

  rotate();

  File f = SPIFFS.open(curPath, "a");
  f.write((uint8_t *)msg, sizeof(*msg));
  f.close();
}

size_t FileLogger::readRows(LogLine* output, time_t time, size_t startIdx, size_t maxCount) {
    char path[32];
    pathFromDate(path, sizeof(path), time);

    if (!SPIFFS.exists(path))
      return 0;

    File f = SPIFFS.open(path, "r");
    maxCount = min((int32_t)maxCount, (int32_t)(f.size() / sizeof(LogLine)) - (int32_t)startIdx);
    if (maxCount <= 0)
      return maxCount;

    f.seek(startIdx * sizeof(LogLine));
    f.read((uint8_t *)output, maxCount * sizeof(LogLine));
    f.close();
    return maxCount;
}

time_t FileLogger::dateFromPath(const char* path) {
  struct tm tm = {0};
  strptime(path, "%Y-%m-%d.log", &tm);
  return mktime(&tm);
}

void FileLogger::rotate() {
  const time_t now = network.getEpochTime();
  const time_t localToday = now / 86400 * 86400;

  if (today != localToday) {
    today = localToday;
    pathFromDate(curPath, sizeof(curPath), now);

    File dir = SPIFFS.open(directory);
    File file = dir.openNextFile();

    while (file) {
      if (!file.isDirectory()) {
        const char *dateStart = file.name();
        const time_t midnight = dateFromPath(dateStart);

        if (midnight < today - daysToKeep * 86400) {
          SPIFFS.remove(file.path());
        }
      }

      file = dir.openNextFile();
    }
  }
}

size_t FileLogger::rowCount(time_t time) {
    char path[32];
    pathFromDate(path, sizeof(path), time);

    if (!SPIFFS.exists(path))
        return 0;

    File f = SPIFFS.open(path, "r");
    const size_t rows = rowCount(f);
    f.close();

    return rows;
}

size_t FileLogger::rowCount(File &logFile) {
    return logFile.size() / sizeof(LogLine);
}

void FileLogger::pathFromDate(char *output, size_t size, time_t time) {
  struct tm *ptm = gmtime(&time);

  char buffer[32];
  strftime(buffer, sizeof(buffer), "%F", ptm);
  snprintf(output, size, pathFormat, directory, buffer);
}

///// Global object

FileLogger fileLog;
