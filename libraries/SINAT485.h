#include "Arduino.h"

class SINAT485 {
  public:
    SINAT485(Uart* incloSer);
    void begin();

  private:
    Uart* _incloSer;
};
