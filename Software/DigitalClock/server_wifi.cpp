#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
#include <FS.h>                               // Please read the instructions on http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system
#include "server_wifi.h"

#define WIFIMODE 1                            // 0 = Only Soft Access Point, 1 = Only connect to local WiFi network with UN/PW, 2 = Both

#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2)
  const char* APssid = "CLOCK_AP";        
  const char* APpassword = "1234567890";  
#endif
  
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2)
  #include "Credentials.h"                    // Create this file in the same directory as the .ino file and add your credentials (#define SID YOURSSID and on the second line #define PW YOURPASSWORD)
  const char *ssid = SID;
  const char *password = PW;
#endif
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdateServer;

SClockData s_clock_data;
bool update_timedate;
bool update_countdown;
bool update_temperature;
bool update_hourformat;
bool update_clock;

ServerWifi::ServerWifi()
{
  update_timedate = false;
  update_countdown = false;
  update_temperature = false;
  update_hourformat = false;
  update_clock = false;

  s_clock_data.r_val = 0;
  s_clock_data.g_val = 255;
  s_clock_data.b_val = 0;
  s_clock_data.brightness = 10;
  s_clock_data.temperatureCorrection = -3.0;
  s_clock_data.temperatureSymbol = 12;                  // 12=Celcius, 13=Fahrenheit check 'numbers'
  s_clock_data.hourFormat = 24;                         // Change this to 12 if you want default 12 hours format instead of 24  
}

void ServerWifi::setup(CRGB *LEDs)
{
    WiFi.setSleepMode(WIFI_NONE_SLEEP); 
    delay(200);

    // WiFi - AP Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2) 
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(APssid, APpassword);    // IP is usually 192.168.4.1
  Serial.println();
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
#endif

  start_wifi(LEDs);   

  httpUpdateServer.setup(&server);

  // Handlers
  //server.on("/color", HTTP_POST, prueba);
  server.on("/color", HTTP_POST, []() {    
    s_clock_data.r_val = server.arg("r").toInt();
    s_clock_data.g_val = server.arg("g").toInt();
    s_clock_data.b_val = server.arg("b").toInt();
    server.send(200, "text/json", "{\"result\":\"ok\"}");
  });

  server.on("/setdate", HTTP_POST, []() { 
    // Sample input: date = "Dec 06 2009", time = "12:34:56"
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    String datearg = server.arg("date");
    String timearg = server.arg("time");
    Serial.println(datearg);
    Serial.println(timearg);    
    server.send(200, "text/json", "{\"result\":\"ok\"}");
    s_clock_data.date = datearg;
    s_clock_data.time = timearg;
    update_timedate = true;
  });

  server.on("/brightness", HTTP_POST, []() {    
    s_clock_data.brightness = server.arg("brightness").toInt();    
    server.send(200, "text/json", "{\"result\":\"ok\"}");
    Serial.println(s_clock_data.brightness);
  });
  
  server.on("/countdown", HTTP_POST, []() {    
    s_clock_data.countdownMilliSeconds = server.arg("ms").toInt();     
    s_clock_data.cd_r_val = server.arg("r").toInt();
    s_clock_data.cd_g_val = server.arg("g").toInt();
    s_clock_data.cd_b_val = server.arg("b").toInt();
    server.send(200, "text/json", "{\"result\":\"ok\"}");
    update_countdown = true;
  });

  server.on("/temperature", HTTP_POST, []() {   
    s_clock_data.temperatureCorrection = server.arg("correction").toInt();
    s_clock_data.temperatureSymbol = server.arg("symbol").toInt();    
    server.send(200, "text/json", "{\"result\":\"ok\"}");
    update_temperature = true;
  });  

  server.on("/hourformat", HTTP_POST, []() {   
    s_clock_data.hourFormat = server.arg("hourformat").toInt();     
    server.send(200, "text/json", "{\"result\":\"ok\"}");
    update_hourformat = true;
  }); 

  server.on("/clock", HTTP_POST, []() {          
    server.send(200, "text/json", "{\"result\":\"ok\"}");
    update_clock = true;
  });  

   // Before uploading the files with the "ESP8266 Sketch Data Upload" tool, zip the files with the command "gzip -r ./data/" (on Windows I do this with a Git Bash)
  // *.gz files are automatically unpacked and served from your ESP (so you don't need to create a handler for each file).
  server.serveStatic("/", SPIFFS, "/", "max-age=86400");
  server.begin();     

  if (!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS contents:");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
  }
  Serial.println(); 

}

void ServerWifi::process(SClockData & data)
{
    server.handleClient();
    data =s_clock_data;
}


bool ServerWifi::is_new_timedate(void)
{
    if (update_timedate)
    {
        update_timedate = false;
        return true;
    }
    return false;
}

bool ServerWifi::is_new_countdown(void)
{
    if (update_countdown)
    {
        update_countdown = false;
        return true;
    }
    return false;
}

bool ServerWifi::is_new_temperature(void)
{
    if (update_temperature)
    {
        update_temperature = false;
        return true;
    }
    return false;
}

bool ServerWifi::is_new_hourformat(void)
{
    if (update_hourformat)
    {
        update_hourformat = false;
        return true;
    }
    return false;
}

bool ServerWifi::is_new_clock(void)
{
    if (update_clock)
    {
        update_clock = false;
        return true;
    }
    return false;
}

void ServerWifi::start_wifi(CRGB *LEDs)
{
   // WiFi - Local network Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2) 
  byte count = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    // Stop if cannot connect
    if (count >= 60) {
      Serial.println("Could not connect to local WiFi.");      
      return;
    }
       
    delay(500);
    Serial.print(".");
    LEDs[count] = CRGB::White;
    FastLED.show();
    count++;
  }
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  IPAddress ip = WiFi.localIP();

#endif
}
