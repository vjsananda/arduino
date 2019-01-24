/* ESP8266 Standalone Alarm prototype version 3
 *  Changes from Version 2
 *  - Moved function to send post request with sensor data to DataXchange tab. 
 *  - Remove reset button and replace with test button. Test button creates reset pulse via ATTINY85.
 *  - Factory reset button connected to GPIO12, when sampled 0 on reset, will erase wifi credentials and open config portal
 *  - Store ID and Encryption password in JSON via Filestore.
 *  - Blink ESP builtin LED (GPIO1, of D1) via Ticker library, during wifi config
 *  - Use WifiManager to configure wifi access and set ID and encryption passord.
 *  
 *  
 *  
 *  =================================================================================
 *  These are v2 notes
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
#include <FS.h>

//Needed for Wifi manager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>   

#include <ArduinoJson.h>
#include "Crypto.h"
#include "ESPBase64.h"
#include <ESP8266WiFi.h>

ADC_MODE(ADC_VCC); //vcc read-mode

//Define Pin numbers
#define WAKE_EVENT_TYPE_PIN 14
#define FACTORY_RESET_PIN   12
//#define STATUS_LED_PIN      LED_BUILTIN
//#define STATUS_LED_PIN      13
#define WAKE_DEEP_SLEEP_PIN      13

#define HOST_NAME   "h2ando.herokuapp.com"
#define HOST_PORT    80
//#define HOST_NAME   "192.168.1.13"
//#define HOST_PORT    3000

#define ACTIVE_LOW  0
#define TIME_BETWEEN_TEXT_MESSAGES 2 //in seconds
#define CLK_DIV 1
#define DBG_PRINT 1
#define MAX_BODY_LENGTH 32 //Make it a multiple of 16, since AES does stuff in 16 byte blocks
#define SERIAL_RX_TIMEOUT 5000 //in milliseconds

#define MAX_STRING_LENGTH 400

#define SLEEP_TIME_IF_ERROR 10

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
#define RTC_BASE 64
#define RTC_STATE 65
#define RTC_PING_COUNT 66
#define RTC_MAIL_TYPE 67
#define RTC_PING_INTERVAL 68
#define RTC_DEFAULT_KEY  70
#define RTC_CURR_KEY  74
#define RTC_PREV_KEY  78

typedef struct {
  uint8_t keybytes[16];
}  key16byte ;

byte state;   // state variable
byte event = 0;
uint32_t pingCount;
uint32_t time1, time2;
float vcc ;
uint32_t pingInterval ;

// WiFi settings
#include "wifi_credentials.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Time to sleep (in seconds):
uint32_t sleepTimeS = 10;

// Host
//const char* host = "dweet.io";
//const char* host = "h2ando.herokuapp.com";
const char* host = HOST_NAME ;
const int   hport = HOST_PORT ;

volatile byte wake_event_type = 0;

WiFiClient client;
char b64data[MAX_STRING_LENGTH];

uint8_t buf[MAX_STRING_LENGTH] ;
uint8_t outbuf[MAX_STRING_LENGTH] ;
char cbuf[MAX_STRING_LENGTH];

char buffer[MAX_STRING_LENGTH];

byte byteBuf[64];

//16 byte arrays
uint8_t key[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t iv[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//define your default values here, if there are different values in config.json, they are overwritten.
char sensor_id[30] ;
char iv_password[30] ;
int ID ;

void updateSleepTime(uint32_t _in) {
  sleepTimeS = _in ;
 }

void WiFiConnect() {
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

//Returns 1 on successful connection
int TCPConnect( int port ) {
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return 0;
  }
  return 1;
}

int FactoryReset ;
int wake_from_deep_sleep ;

void setup()
{
  pinMode(WAKE_EVENT_TYPE_PIN, INPUT_PULLUP);
  pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
  //pinMode(STATUS_LED_PIN,OUTPUT);
  pinMode(WAKE_DEEP_SLEEP_PIN, INPUT_PULLUP);
  
  wake_event_type = digitalRead(WAKE_EVENT_TYPE_PIN);
  wake_from_deep_sleep = digitalRead(WAKE_DEEP_SLEEP_PIN);
  
  Serial.begin(115200);
  FactoryReset = 0;

  Serial.printf("\n** Wake Event Type = %d **\n\n",wake_event_type);
  Serial.printf("** Wake Deep Sleep  = %d **\n\n",wake_from_deep_sleep);

  //Factory reset
  //Blow away saved parameters, ssid , password etc
  if (digitalRead(FACTORY_RESET_PIN) == ACTIVE_LOW) {
    Serial.println("*** Factory Reset ***");
    FactoryReset=1;
    //clean FS, for testing
    SPIFFS.format();

    //Disconnect WiFi
    WiFi.disconnect();
  }

  system_rtc_mem_read(RTC_BASE, buf, 2); // read 2 bytes from RTC-MEMORY

  if ((buf[0] != 0x55) || (buf[1] != 0xaa))  // cold start, magic number is nor present
  { state = STATE_COLDSTART;
    buf[0] = 0x55; buf[1] = 0xaa;
    system_rtc_mem_write(RTC_BASE, buf, 2);
  }
  else // reset was due to sleep-wake or external event
  { system_rtc_mem_read(RTC_STATE, buf, 1);
    state = buf[0];
    if (state == STATE_SLEEP_WAKE) // could be a sleep wake or an alarm
    {
      wake_event_type = digitalRead(WAKE_EVENT_TYPE_PIN);
    }
  }

  switch (state) {
    case STATE_COLDSTART:
      pingCount = 0;
      system_rtc_mem_write(RTC_PING_COUNT, &pingCount, 4); // initialize counter
      break;

    case STATE_SLEEP_WAKE:
      system_rtc_mem_read(RTC_PING_COUNT, &pingCount, 4); // read counter
      pingCount++;
      system_rtc_mem_write(RTC_PING_COUNT, &pingCount, 4); // initialize counter
      break;
  }

  //WiFiConnect();
  configureWiFi();

  readParam();

  ID=atoi(sensor_id);
  Serial.print("Sensor ID = ");
  Serial.println(ID);

  Serial.println("ESP8266-Standalone in normal mode");

  // Logging data to cloud
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections

  const int httpPort = hport ;
  
  if ( TCPConnect(httpPort) ) {
    vcc = ESP.getVcc();
  
    delay(10);
  
    String postMsg ;
    //wake_event_type = 1 when alarm
    postMsg = getJsonString(ID, 1, vcc, wake_event_type );
    
    #ifdef DBG_PRINT
    Serial.println("postMsg = " + postMsg);
    #endif
    
    sendPostRequest(postMsg);
  
    #ifdef DBG_PRINT
    Serial.println();
    Serial.println("closing connection");
    
    // Sleep
    Serial.println("ESP8266 in sleep mode");
    #endif
    
    buf[0] = STATE_SLEEP_WAKE;
    system_rtc_mem_write(RTC_STATE, buf, 1); // set state for next wakeUp
  
    delay(500);
  }

  //Sleep time in microseconds
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop()
{
  //Doesn't wake from deep sleep without reset
  Serial.println("Waking from sleep..");
  ESP.deepSleep(sleepTimeS * 1000000);
}
