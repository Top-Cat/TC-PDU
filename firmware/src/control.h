#include "output.h"

class PDUControl {
  public:
    static void setupTask();
    void task();
    uint32_t getUptime();
    float getTotalPower();
  private:
    float totalPower = 0;

    Output* getLowestPriority();
};

extern PDUControl control;