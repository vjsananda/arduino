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
//#include <ESP8266_AESLib.h>
#include "ESPBase64.h"
#include "aes256.h" //Include library files
#include "libb64/cdecode.h"
#include "libb64/cencode.h"

#include <ESP8266WiFi.h>
ADC_MODE(ADC_VCC); //vcc read-mode

//#define HOST_NAME   "h2ando.herokuapp.com"
//#define HOST_PORT    80
#define HOST_NAME   "192.168.1.23"
#define HOST_PORT    3000

#define TIME_BETWEEN_TEXT_MESSAGES 2 //in seconds
#define CLK_DIV 1
#define DBG_PRINT 1
//#define PING_INTERVAL 1440 //in watchdog intervals, max watchdog time = 8 sec
#define BUFFER_LENGTH 50
#define MAX_BODY_LENGTH 32 //Make it a multiple of 16, since AES does stuff in 16 byte blocks
#define SERIAL_RX_TIMEOUT 5000 //in milliseconds

#define MAX_STRING_LENGTH 400

uint8_t buf[MAX_STRING_LENGTH] ;
char cbuf[MAX_STRING_LENGTH];

char buffer[BUFFER_LENGTH];

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

byte state;   // state variable
byte event = 0;
uint32_t pingCount;
uint32_t time1, time2;
float vcc ;

#define WAKE_EVENT_TYPE_PIN 14

// WiFi settings
#include "wifi_credentials.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

//32 bit ID of this node
const unsigned int ID = 0xdeadbeef;

// Time to sleep (in milliseconds):
const int sleepTimeMS = 2000;

// Time to sleep (in seconds):
const int sleepTimeS = 10;

// Host
//const char* host = "dweet.io";
//const char* host = "h2ando.herokuapp.com";
const char* host = HOST_NAME ;
const int   hport = HOST_PORT ;

volatile byte wake_event_type = 0;

WiFiClient client;
aes256_context ctxt;
char b64data[BUFFER_LENGTH];

int test_post_request(String body) {

  int start_idx ;
  int end_idx ;
  int len ;
  int i , j;
  String padStr , msg;
  String randKey ;
  uint8_t key[32];
  int keylen ;

  StaticJsonBuffer<75> jsonBuffer;

  randKey = genRandomKey();
  //keylen = stringToUintBuf(randKey, key);
  for(i=0;i<32;i++)
    key[i]  = 0;

  aes256_init(&ctxt, key);

  Serial.println( "len of body = " + String(body.length()) ) ;
  
  padStr = pad16Byte(body);
  Serial.println( "padStr = " + padStr ) ;
  Serial.println( "len of padStr = " + String(padStr.length()) ) ;

  len = stringToUintBuf(padStr, buf);

  for (int i = 0; i < len; i += 16)
    aes256_encrypt_ecb(&ctxt, buf + i );

  //Takes uint_8 buf and converts to base64 encoded string
  int b64len = base64_encode(b64data, (char *)buf, len);
  String encrypted = String(b64data);

  //String encrypted = uintBufToString(buf, len);
  Serial.println("encrypted = " + encrypted );
  Serial.println( "len of encrypted = " + String(encrypted.length()) ) ;
  
//  len = stringToUintBuf(encrypted, buf);
//  for (int i = 0; i < len; i++)
//    Serial.println(buf[i],HEX);

  String preamble = "POST /postform HTTP/1.0\r\nhost:" + String(HOST_NAME) + "\r\n";
  //String header = "content-length:" + String(MAX_BODY_LENGTH) + "\r\n" ;
  String header = "content-length:" + String(encrypted.length()) + "\r\n" ;
  String blank_line = "\r\n";

  //for(i=0;i<body.length();i++)
  //encrypted_body[i] = body.c_str()[i];

  //aes128_cbc_enc(key, key, encrypted_body, MAX_BODY_LENGTH);

  //String POST_REQUEST = preamble + header + blank_line + String((char *)encrypted_body) + blank_line + blank_line ;
  String POST_REQUEST = preamble + header + blank_line + encrypted + blank_line + blank_line ;

#ifdef DBG_PRINT
  Serial.print(POST_REQUEST);
  Serial.println("");
  Serial.print("REQUEST length = ");
  Serial.println(POST_REQUEST.length());
#endif

  client.print(POST_REQUEST);
  //wifi.send((const uint8_t*)POST_REQUEST.c_str(), POST_REQUEST.length());

  //wifi.recvEcho(SERIAL_RX_TIMEOUT);
  //wifi.recvRaw(buffer, BUFFER_LENGTH, '{', SERIAL_RX_TIMEOUT ) ;
  //#ifdef DBG_PRINT
  //Serial.println(buffer);
  //#endif
  //WifiPowerDown();

  delay(100);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  /*
  JsonObject & root = jsonBuffer.parseObject(buffer);

   if (!root.success())
   {
     Serial.println("parseObject() failed");
     return 0;
   }

   int e,m,cmd;
   int p;

   //i = root["i"];
   e = root["e"];
   m = root["m"];
   cmd = root["c"];
   p = root["p"];

   //Serial.print("i=");
   //Serial.println(i);

   #ifdef DBG_PRINT
   Serial.print("e=");
   Serial.println(e);

   Serial.print("m=");
   Serial.println(m);

   Serial.print("c=");
   Serial.println(cmd);

   Serial.print("p=");
   Serial.println(p);

   Serial.flush();
   #endif

   //If no error then ping after ping interval
   if (e == 0)
     return p;
   else
     return e;
     */

}

void alarm_wakeup() {
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

void TCPConnect( int port ) {
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
}

void setup()
{
  pinMode(WAKE_EVENT_TYPE_PIN, INPUT_PULLUP);

  //attachInterrupt( WAKE_EVENT_TYPE_PIN , alarm_wakeup, LOW);

  // Serial
  Serial.begin(115200);
  Serial.println("ESP8266-Standalone in normal mode");

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

  WiFiConnect();

  // Print the IP address
  Serial.println(WiFi.localIP());
  Serial.println("ESP8266-Standalone in normal mode");

  // Logging data to cloud
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections

  const int httpPort = hport ;
  TCPConnect(httpPort);

  vcc = ESP.getVcc();

  /*
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
  */

  delay(10);

  /*
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  */

  test_post_request( "{v:4,p:10,msg:'i love dogs'}");

  Serial.println();
  Serial.println("closing connection");

  // Sleep
  Serial.println("ESP8266 in sleep mode");

  buf[0] = STATE_SLEEP_WAKE;
  system_rtc_mem_write(RTC_STATE, buf, 1); // set state for next wakeUp

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
