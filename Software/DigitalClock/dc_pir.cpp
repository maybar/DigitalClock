#include "dc_pir.h"
#include <ESP8266WiFi.h>
#define timeSeconds 600

DcPir::DcPir(int const pin)
{
  _i_pin = pin;
  _ul_last_trigger = 0;
  _b_start_timer = false;
  _i_old_pir_status = 0;
  _b_pir_state = 0; //false: IDLE, true: MOV
}

void DcPir::setup()
{
  pinMode(_i_pin, INPUT);    //set again because some library rewrites the setting
}

bool DcPir::process()
{
  unsigned long now = millis();
  int const pir_status = digitalRead(_i_pin);
  
  //low = no motion, high = motion
  if ( _i_old_pir_status != pir_status)
  {
    Serial.println(pir_status);
    if (pir_status == 0)
    {
      _b_start_timer = true;
      _ul_last_trigger = millis();
    }
    else
    {
      Serial.println("Motion detected  ALARM : ");
      _b_start_timer = false;
      _b_pir_state = true;
    }
    _i_old_pir_status = pir_status;
  }
  if(_b_start_timer && ((now - _ul_last_trigger) > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    _b_start_timer = false;
    _b_pir_state = false;
  }
  return _b_pir_state;
}
