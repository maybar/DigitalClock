struct SClockData{
    byte r_val;
    byte g_val;
    byte b_val;
    String date;
    String time;
    byte brightness;  
    unsigned long countdownMilliSeconds;
    byte cd_r_val;
    byte cd_g_val;
    byte cd_b_val;
    float temperatureCorrection;
    byte temperatureSymbol;                  // 12=Celcius, 13=Fahrenheit check 'numbers'
    byte hourFormat;                         // Change this to 12 if you want default 12 hours format instead of 24  
};
