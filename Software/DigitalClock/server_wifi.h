#ifndef SERVER_WIFI_H // include guard
#define SERVER_WIFI_H
#include "s_clock_data.h"

class ServerWifi
{
  public:
    ServerWifi();
    void setup(CRGB *LEDs);
    void process(SClockData & data);
    bool is_new_timedate(void);
    bool is_new_countdown(void);
    bool is_new_temperature(void);
    bool is_new_hourformat(void);
    bool is_new_clock(void);

  private:
    void start_wifi(CRGB *LEDs);
};


#endif /* SERVER_WIFI_H */
