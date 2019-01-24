/* ESP8266 Standalone Alarm prototype
 *  Use Generic-ESP8266-Module
 *  Works in conjunction with ATMEGA328 standalone for event detection
 *  mkZ-Arduino-Interrupt-Router sketch.
 * 
 * ESP goes to deepsleep (current ~ 18 uA), to wake up the RTC sends out 
 * an active low pulse on GPIO16. This is routed to the atmega which 
 * distinguishes between the wake up pulse and an actual alarm event.
 * 
 * The wake event type information is sampled by the ESP8266 on pin WAKE_EVENT_TYPE_PIN 
 * (set at pin 14 currently)
 * 
 */
// Library
#include <ArduinoJson.h>
#include "wifi_credentials.h"

#include <ESP8266WiFi.h>
ADC_MODE(ADC_VCC); //vcc read-mode

// Include API-Headers
extern "C" {
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "cont.h"
} 

// state definietions
#define STATE_COLDSTART 0
#define STATE_SLEEP_WAKE 1
#define STATE_ALARM 2
#define STATE_CONNECT_WIFI 4

// RTC-MEM Adresses
#define RTC_BASE 65
#define RTC_STATE 66
#define RTC_PING_COUNT 67
#define RTC_MAIL_TYPE 68 

// global variables
byte buf[10];
byte state;   // state variable
byte event = 0;
uint32_t pingCount;
uint32_t time1, time2;
float vcc ;

#define WAKE_EVENT_TYPE_PIN 14

// WiFi settings
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Time to sleep (in milliseconds):
const int sleepTimeMS = 2000;

// Time to sleep (in seconds):
const int sleepTimeS = 10;

// Host
const char* host = "dweet.io";
volatile byte wake_event_type = 0;

void alarm_wakeup() {
  
}

void setup() 
{
  pinMode(WAKE_EVENT_TYPE_PIN,INPUT_PULLUP);

  //attachInterrupt( WAKE_EVENT_TYPE_PIN , alarm_wakeup, LOW);
  
   
  // Serial
  Serial.begin(115200);
  Serial.println("ESP8266-Standalone in normal mode");

   system_rtc_mem_read(RTC_BASE,buf,2); // read 2 bytes from RTC-MEMORY

   if ((buf[0] != 0x55) || (buf[1] != 0xaa))  // cold start, magic number is nor present
   { state = STATE_COLDSTART;
     buf[0] = 0x55; buf[1]=0xaa;
     system_rtc_mem_write(RTC_BASE,buf,2);
   } 
   else // reset was due to sleep-wake or external event
   {  system_rtc_mem_read(RTC_STATE,buf,1);
      state = buf[0];
      if (state == STATE_SLEEP_WAKE) // could be a sleep wake or an alarm
      {  
        wake_event_type = digitalRead(WAKE_EVENT_TYPE_PIN);
      }
   }

  switch(state) {
    case STATE_COLDSTART:
     pingCount = 0;
     system_rtc_mem_write(RTC_PING_COUNT,&pingCount,4);  // initialize counter
     break;

     case STATE_SLEEP_WAKE:
     system_rtc_mem_read(RTC_PING_COUNT,&pingCount,4);  // read counter
     pingCount++;
     system_rtc_mem_write(RTC_PING_COUNT,&pingCount,4);  // initialize counter
     break;
  }
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Print the IP address
  Serial.println(WiFi.localIP());
  Serial.println("ESP8266-Standalone in normal mode");
  
  // Logging data to cloud
  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  vcc = ESP.getVcc();
  
  if (wake_event_type) {
    
  // This will send the request to the server
  client.print(String("GET /dweet/for/mkz?event=ALARM&count=") + String(pingCount) + "&vcc=" + String(vcc) +  " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  }
  else {
    
      client.print(String("GET /dweet/for/mkz?event=PING&count=") + String(pingCount) + "&vcc=" + String(vcc) + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  }
  
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");

  // Sleep
  Serial.println("ESP8266 in sleep mode");

  buf[0] = STATE_SLEEP_WAKE;  
  system_rtc_mem_write(RTC_STATE,buf,1); // set state for next wakeUp
          
  delay(500);

  //Sleep time in microseconds
  //ESP.deepSleep(sleepTimeS * 1000000);

  ESP.deepSleep(sleepTimeMS * 1000);
  
}

void loop() 
{
  //Doesn't wake from deep sleep without reset
  Serial.println("Waking from sleep..");
  //ESP.deepSleep(sleepTimeS * 1000000);
  ESP.deepSleep(sleepTimeMS * 1000);
}
