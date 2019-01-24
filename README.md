#Wifi credentials are in a header file for each sketch
wifi_credentials.h
This file is not uploaded to the public git repo
for obvious reasons
This file defines
WIFI_SSID & WIFI_PASSWORD
To compile a sketch create this file in the same directory
as the sketch
#define WIFI_SSID     <your_wifi_ssid>
#define WIFI_PASSWORD <your_wifi_password>

# Arduino: Description of sketches
atmega_esp8266: standalone atmega + aes + new serial parse + json

mkZ-ESP8266-Standalone-Alarm-v? : 4/16/2017
Uses deepsleep in ESP8266 (ESP12) module and ATMEGA328 to route alarm vs wake sleep events.
Works in conjunction with mkZ-Arduino-Interrupt-Router.

mkZ-ESP8266-Standalone-Alarm-v5 : 11/16/2017
Uses deepsleep in ESP8266 (ESP12) module and ATMEGA328 to route alarm vs wake sleep events.
Works in conjunction with mkZ-Arduino-Interrupt-Router-v2.
Feature list:
   * Wifi configuration manager
   * Built-in LED blink during configuration
   * Configuration timeout
   * Support sleep duration larger than max deepSleep duration of 71 min, using RTC mem.
   * Distinguish between Power on reset, Ping reset, and alarm reset.
   * Added DBG_PRINT define. Prints to Serial console only when defined.

Experimental sketches
-atmega_interrupt_router
-ESP8266-Standalone-RouteInterrupt
-ESP8266-Standalone-SleepWake
-ESP8266-Standalone-WifiTest

