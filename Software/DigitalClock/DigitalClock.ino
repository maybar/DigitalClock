#include <FastLED.h>
#include <Wire.h>
#include <RtcDS3231.h>                        // Include RTC library by Makuna: https://github.com/Makuna/Rtc
#define FASTLED_INTERNAL
#include <FastLED.h>
#include "dc_pir.h"
#include "server_wifi.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))
#define NUM_LEDS 30                           // Total of 30 LED's     
#define MILLI_AMPS 500                        // 500mA 
#define COUNTDOWN_OUTPUT D5                   // OUTPUT_PIN (Buzzer)
//D1 -> SCL CLOCK
//D2 -> SDA CLOCK
#define DATA_PIN D4                           // OUTPUT_PIN - Change this if you are using another type of ESP board than a WeMos D1 Mini
#define PIR_PIN   D7                            // INPUT_PIN
#define LDR_SENSOR_PIN A0                     // INPUT PIN

RtcDS3231<TwoWire> Rtc(Wire);
CRGB LEDs[NUM_LEDS];
DcPir dc_pir(PIR_PIN);
ServerWifi server_wifi;

// Settings
unsigned long _ul_last_trigger = 0;
bool buzzer_on = false;
unsigned long prevTime = 0;
byte r_val = 0;
byte g_val = 255;
byte b_val = 0;
bool dotsOn = true;
byte brightness = 255;
float temperatureCorrection = -3.0;
byte temperatureSymbol = 12;                  // 12=Celcius, 13=Fahrenheit check 'numbers'
byte clockMode = 0;                           // Clock modes: 0=Clock, 1=Countdown, 2=Temperature, 3=Scoreboard
unsigned long countdownMilliSeconds;
unsigned long endCountDownMillis;
byte hourFormat = 24;                         // Change this to 12 if you want default 12 hours format instead of 24               
CRGB countdownColor = CRGB::Green;
byte scoreboardLeft = 0;
byte scoreboardRight = 0;
CRGB scoreboardColorLeft = CRGB::Green;
CRGB scoreboardColorRight = CRGB::Red;
CRGB alternateColor = CRGB::Black; 
long numbers[] = {
  0b0111111,  // [0] 0
  0b0001100,  // [1] 1
  0b1011011,  // [2] 2
  0b1011110,  // [3] 3
  0b1101100,  // [4] 4
  0b1110110,  // [5] 5
  0b1110111,  // [6] 6
  0b0011100,  // [7] 7
  0b1111111,  // [8] 8
  0b1111110,  // [9] 9
  0b0000000,  // [10] off
  0b1111000,  // [11] degrees symbol
  0b0011110,  // [12] C(elsius)
  0b1011100,  // [13] F(ahrenheit)
};

uint8_t hora=0;
uint8_t minuto=0;
uint8_t segundo = 0;

void updateClock();
void updateCountdown();
void updateTemperature();
void updateScoreboard();
void displayDots(CRGB color);
void wifi_client();
void light_sensor();

void setup() {
  pinMode(COUNTDOWN_OUTPUT, OUTPUT);
  Serial.begin(115200); 
  delay(200);

  // RTC DS3231 Setup
  Rtc.Begin();    
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid()) {
      if (Rtc.LastError() != 0) {
          // we have a communications error see https://www.arduino.cc/en/Reference/WireEndTransmission for what the number means
          Serial.print("RTC communications error = ");
          Serial.println(Rtc.LastError());
      } else {
          // Common Causes:
          //    1) first time you ran and the device wasn't running yet
          //    2) the battery on the device is low or even missing
          Serial.println("RTC lost confidence in the DateTime!");
          // following line sets the RTC to the date & time this sketch was compiled
          // it will also reset the valid flag internally unless the Rtc device is
          // having an issue
          Rtc.SetDateTime(compiled);
      }
  }

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(LEDs, NUM_LEDS);  
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(LEDs, NUM_LEDS, CRGB::Black);
  FastLED.show();

  server_wifi.setup(&LEDs[0]);

  //Serial.setDebugOutput(true);

  dc_pir.setup();
  pinMode(LDR_SENSOR_PIN, INPUT);

  digitalWrite(COUNTDOWN_OUTPUT, HIGH);
  delay(3000);
  digitalWrite(COUNTDOWN_OUTPUT, LOW);
 
}

void loop(){
  // WIFI
  wifi_client();
  
  //CLOCK
  unsigned long currentMillis = millis();  
  if (currentMillis - prevTime >= 1000) {
    prevTime = currentMillis;

    if (clockMode == 0) {
      updateClock();
    } else if (clockMode == 1) {
      updateCountdown();
    } else if (clockMode == 2) {
      updateTemperature();      
    } else if (clockMode == 3) {
      updateScoreboard();            
    }

    // Light sensor
    light_sensor();
    
    FastLED.setBrightness(brightness);
    FastLED.show();
  }   

  // PIR
  /*boolean const pir_state = dc_pir.process();
  if (pir_state == false)
  {
    brightness = if_day()?BRIGHT_IDLE:BRIGHT_OFF;
  }else
  {
    brightness = if_day()?BRIGHT_HI:BRIGHT_LO;
  }*/

  // BUZZER
  if (buzzer_on)
  {
    if ((currentMillis - _ul_last_trigger) > (2000))
    {
      if (digitalRead(COUNTDOWN_OUTPUT))
        digitalWrite(COUNTDOWN_OUTPUT, LOW);
      else
        digitalWrite(COUNTDOWN_OUTPUT, HIGH);
      _ul_last_trigger = millis();
    }
  }
  else
  {
    digitalWrite(COUNTDOWN_OUTPUT, LOW);
  }
}

void light_sensor()
{
  int sensorValue = analogRead(LDR_SENSOR_PIN);
  // float voltage = sensorValue * (3.2 / 1023.0);
  // int i_vol = int(voltage * 10);
  brightness = map(sensorValue, 0, 1023, 10, 100);
  // Serial.println(sensorValue);
  //Serial.println(brightness);
  
}

bool if_day()
{
  if ((hora > 6) && (hora < 18))
    return true;
  else
    return false;
}


void displayNumber(byte number, byte segment, CRGB color) {
 /*
 * 
    _4         __            __        __  
  _5   _3    __   __   14  __  __   __    __
    _6         __            __        __  
  _0   _2    _7   __   15  16  __   23    __
    _1         __            __        __   

 */
 
  // segment from left to right: 3, 2, 1, 0
  byte startindex = 0;
  switch (segment) {
    case 0:
      startindex = 23;
      break;
    case 1:
      startindex = 16;
      break;
    case 2:
      startindex = 7;
      break;
    case 3:
      startindex = 0;
      break;    
  }

  for (byte i=0; i<7; i++){
    yield();
    LEDs[i + startindex] = ((numbers[number] & 1 << i) == 1 << i) ? color : alternateColor;
  } 
}

void allBlank() {
  for (int i=0; i<NUM_LEDS; i++) {
    LEDs[i] = CRGB::Black;
  }
  FastLED.show();
}

void updateClock() {  
  RtcDateTime now = Rtc.GetDateTime();
  //printDateTime(now);    

  int hour = now.Hour();
  int mins = now.Minute();
  int secs = now.Second();

  hora = hour;
  minuto = mins;
  segundo = secs;

  if (hourFormat == 12 && hour > 12)
    hour = hour - 12;
  
  byte h1 = hour / 10;
  byte h2 = hour % 10;
  byte m1 = mins / 10;
  byte m2 = mins % 10;  
  byte s1 = secs / 10;
  byte s2 = secs % 10;
  
  CRGB color = CRGB(r_val, g_val, b_val);

  if (h1 > 0)
    displayNumber(h1,3,color);
  else 
    displayNumber(10,3,color);  // Blank
    
  displayNumber(h2,2,color);
  displayNumber(m1,1,color);
  displayNumber(m2,0,color); 

  displayDots(color);  
}

void updateCountdown() {

  if (countdownMilliSeconds == 0 && endCountDownMillis == 0) 
    return;
    
  unsigned long restMillis = endCountDownMillis - millis();
  unsigned long hours   = ((restMillis / 1000) / 60) / 60;
  unsigned long minutes = (restMillis / 1000) / 60;
  unsigned long seconds = restMillis / 1000;
  int remSeconds = seconds - (minutes * 60);
  int remMinutes = minutes - (hours * 60); 
  
  Serial.print(restMillis);
  Serial.print(" ");
  Serial.print(hours);
  Serial.print(" ");
  Serial.print(minutes);
  Serial.print(" ");
  Serial.print(seconds);
  Serial.print(" | ");
  Serial.print(remMinutes);
  Serial.print(" ");
  Serial.println(remSeconds);

  byte h1 = hours / 10;
  byte h2 = hours % 10;
  byte m1 = remMinutes / 10;
  byte m2 = remMinutes % 10;  
  byte s1 = remSeconds / 10;
  byte s2 = remSeconds % 10;

  CRGB color = countdownColor;
  if (restMillis <= 60000) {
    color = CRGB::Red;
  }

  if (hours > 0) {
    // hh:mm
    displayNumber(h1,3,color); 
    displayNumber(h2,2,color);
    displayNumber(m1,1,color);
    displayNumber(m2,0,color);  
  } else {
    // mm:ss   
    displayNumber(m1,3,color);
    displayNumber(m2,2,color);
    displayNumber(s1,1,color);
    displayNumber(s2,0,color);  
  }

  displayDots(color);  

  if (hours <= 0 && remMinutes <= 0 && remSeconds <= 0) {
    Serial.println("Countdown timer ended.");
    //endCountdown();
    countdownMilliSeconds = 0;
    endCountDownMillis = 0;
    buzzer_on = true;
    _ul_last_trigger = millis();
    return;
  }  
}

void endCountdown() {
  allBlank();
  for (int i=0; i<NUM_LEDS; i++) {
    if (i>0)
      LEDs[i-1] = CRGB::Black;
    
    LEDs[i] = CRGB::Red;
    FastLED.show();
    delay(25);
  }  
}

void displayDots(CRGB color) {
  if (dotsOn) {
    LEDs[14] = color;
    LEDs[15] = color;
  } else {
    LEDs[14] = CRGB::Black;
    LEDs[15] = CRGB::Black;
  }

  dotsOn = !dotsOn;  
}

void hideDots() {
  LEDs[14] = CRGB::Black;
  LEDs[15] = CRGB::Black;
}

void updateTemperature() {
  RtcTemperature temp = Rtc.GetTemperature();
  float ftemp = temp.AsFloatDegC();
  float ctemp = ftemp + temperatureCorrection;
  Serial.print("Sensor temp: ");
  Serial.print(ftemp);
  Serial.print(" Corrected: ");
  Serial.println(ctemp);

  if (temperatureSymbol == 13)
    ctemp = (ctemp * 1.8000) + 32;

  byte t1 = int(ctemp) / 10;
  byte t2 = int(ctemp) % 10;
  CRGB color = CRGB(r_val, g_val, b_val);
  displayNumber(t1,3,color);
  displayNumber(t2,2,color);
  displayNumber(11,1,color);
  displayNumber(temperatureSymbol,0,color);
  hideDots();
}

void updateScoreboard() {
  byte sl1 = scoreboardLeft / 10;
  byte sl2 = scoreboardLeft % 10;
  byte sr1 = scoreboardRight / 10;
  byte sr2 = scoreboardRight % 10;

  displayNumber(sl1,3,scoreboardColorLeft);
  displayNumber(sl2,2,scoreboardColorLeft);
  displayNumber(sr1,1,scoreboardColorRight);
  displayNumber(sr2,0,scoreboardColorRight);
  hideDots();
}

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}

void wifi_client()
{
  SClockData s_data_clock;
  server_wifi.process(s_data_clock);
  r_val = s_data_clock.r_val;
  g_val = s_data_clock.g_val;
  b_val = s_data_clock.b_val;
  brightness = s_data_clock.brightness;
  hourFormat = s_data_clock.hourFormat;

  if (server_wifi.is_new_timedate())
  {
    char d[12];
    char t[9];
    s_data_clock.date.toCharArray(d, 12);
    s_data_clock.time.toCharArray(t, 9);
    RtcDateTime compiled = RtcDateTime(d, t);
    Rtc.SetDateTime(compiled);   
    clockMode = 0; 
    //hora = compiled.Hour();
    //minuto = compiled.Minute();
    //segundo = compiled.Second();
    buzzer_on = false;   
  }

  if (server_wifi.is_new_countdown())
  {
    buzzer_on =false;
    countdownMilliSeconds = s_data_clock.countdownMilliSeconds;
    countdownColor = CRGB(s_data_clock.cd_r_val, s_data_clock.cd_g_val, s_data_clock.cd_b_val); 
    endCountDownMillis = millis() + countdownMilliSeconds;
    allBlank(); 
    clockMode = 1;     
  }

  if (server_wifi.is_new_temperature())
  {
    temperatureCorrection = s_data_clock.temperatureCorrection;
    temperatureSymbol = s_data_clock.temperatureSymbol;
    clockMode = 2; 
  }

  if (server_wifi.is_new_hourformat() or server_wifi.is_new_clock())
  {
    temperatureCorrection = s_data_clock.temperatureCorrection;
    temperatureSymbol = s_data_clock.temperatureSymbol;
    clockMode = 0;
    buzzer_on = false;
  }
}
