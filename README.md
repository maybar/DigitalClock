# DigitalClock
## Wall Digital Clock 

Conexiones
|Pin	|Dir	|Función 	|  
|-------|-----|-----------------|  
| D0 	| ->  | BUZZER			|  
| D1 	| ->  | SCL CLOCK		|
| D2 	| <-> | SDA CLOCK		|
| D3 	|		|	|
| D4 	| ->  | LED DATA		|
| D5	|	|	|
| D6	|	|	|
| D7 	| <-  | PIR			|
| D8	|	|	|
| A0 	| <-  | LIGHT SENSOR	|


Basado en 
https://github.com/leonvandenbeukel/7-Segment-Digital-Clock-V2

Añadir al gestor URLs de tarjetas de arduino:
https://arduino.esp8266.com/stable/package_esp8266com_index.json


Instalar plugin para subir data a la memoria Flash, y ponerla en la siguinete ruta:
C:\Users\Miguel\Documents\Arduino\Workspace\tools\ESP8266FS\tool\esp8266fs.jar




Consumo
Sin Leds: 80mA  400mW
Brillo
255         300mA
128         200mA
67          155mA
50			130mA
34          119mA
21          111mA
15          110mA
10          110mA


float batteryLevel = map(analogRead(33), 0.0f, 4095.0f, 0, 100);










Clonar la librería para Arduino
git clone https://github.com/esp8266/Arduino.git esp8266/Arduino

