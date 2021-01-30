#ifndef DC_PIR_H // include guard
#define DC_PIR_H

#define BRIGHT_OFF  0
#define BRIGHT_IDLE  10
#define BRIGHT_LO  50
#define BRIGHT_HI  100


class DcPir
{
  public:
    DcPir(int const pin);
    void setup(void);
    bool process(void);
  private:
    unsigned long _ul_last_trigger = 0;
    bool _b_start_timer = false;
    int _i_old_pir_status = 0;
    bool _b_pir_state = 0; //false: IDLE, true: MOV
    int _i_pin;
};

#endif /* DC_PIR_H */
