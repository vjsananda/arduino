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
#include "Crypto.h"
#include "ESPBase64.h"
#include <ESP8266WiFi.h>

ADC_MODE(ADC_VCC); //vcc read-mode

#define HOST_NAME   "h2ando.herokuapp.com"
#define HOST_PORT    80
//#define HOST_NAME   "192.168.1.13"
//#define HOST_PORT    3000

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

#define WAKE_EVENT_TYPE_PIN 14

// WiFi settings
#include "wifi_credentials.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const unsigned int ID = 99;

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

String ivpass = "deadbeeffeedbeef";

//Generate IV array of bytes from string
void generateIV(String str , uint8_t * buf) {
  str.toCharArray(buffer,17);
  for (int i=0;i<16;i++) {
    buf[i] = (uint8_t)buffer[i];  
    if ( i % 2 ) {
      buf[i] = buf[i] % 16 ;
    }
    else {
      buf[i] = 15 - (buf[i] % 16);
    }
  }
}
  
void updateSleepTime(uint32_t _in) {
  sleepTimeS = _in ;
 }

//Return a Json string given input fields
String getJsonString( int id, int msg_id, float vcc, int alarm ) {
    
  StaticJsonBuffer<200> jsonBuffer;
  String JsonString ;

  JsonObject& root = jsonBuffer.createObject();
  root["i"] = id;
  root["v"] = vcc;
  root["a"] = alarm ;
  root["m"] = msg_id;

  root.printTo(JsonString);
  return JsonString;
}

//Encrypt and send post request
int sendPostRequest(String body) {

  int start_idx ;
  int end_idx ;
  int len ;
  int i , j;
  String padStr , msg;
  String randKey ;
  String tmp;
  int keylen ;

  generateIV(ivpass,key);
  generateIV(ivpass,iv);

  Serial.println("iv");
  printUint8Buf(iv,16);
  Serial.println("\n");

  AES AES_Encrypt = AES( key, iv, AES::AES_MODE_128, AES::CIPHER_ENCRYPT);
  AES AES_Decrypt = AES( key, iv, AES::AES_MODE_128, AES::CIPHER_DECRYPT);
  
  DynamicJsonBuffer jsonBuffer;

  #ifdef DBG_PRINT
  Serial.println( "len of body = " + String(body.length()) ) ;
  #endif
  
  padStr = pad16Byte(body);

  #ifdef DBG_PRINT
  Serial.println( "padStr = " + padStr ) ;
  Serial.println( "len of padStr = " + String(padStr.length()) ) ;
  #endif
  
  len = stringToUintBuf(padStr, buf);

  AES_Encrypt.process(buf, outbuf, len);  
  tmp = uintBufToString( outbuf , len );

  #ifdef DBG_PRINT
  Serial.println("Encrypted string = "  + tmp );
  #endif
  
  int b64len = base64_encode(b64data, (char *)outbuf, len) ;
  String encrypted = String(b64data);
  
  #ifdef DBG_PRINT
  Serial.println("Encrypted string (base64) = "  + encrypted );  
  Serial.println( "len of encrypted = " + String(encrypted.length()) ) ;
  #endif

  //Garble and base64 encode the ID
  len = garbleId(8,ID, byteBuf ) ;
  Serial.println("Garbled Id bytebuf = ");
  printByteBuf(byteBuf,len);
  b64len = base64_encode(b64data, (char *)byteBuf, len) ;
  String garbledIdStr = String(b64data);

  String postBody = "{" + garbledIdStr + "}" + "[" + encrypted + "]";
  
  String preamble = "POST /postform HTTP/1.0\r\nhost:" + String(HOST_NAME) + "\r\n";
  //String header = "content-length:" + String(MAX_BODY_LENGTH) + "\r\n" ;
  String header = "content-length:" + String(postBody.length()) + "\r\n" ;
  String blank_line = "\r\n";

  //for(i=0;i<body.length();i++)
  //encrypted_body[i] = body.c_str()[i];

  //aes128_cbc_enc(key, key, encrypted_body, MAX_BODY_LENGTH);

  //String POST_REQUEST = preamble + header + blank_line + String((char *)encrypted_body) + blank_line + blank_line ;
  String POST_REQUEST = preamble + header + blank_line + postBody + blank_line + blank_line ;

#ifdef DBG_PRINT
  Serial.print(POST_REQUEST);
  Serial.println("");
  Serial.print("REQUEST length = ");
  Serial.println(POST_REQUEST.length());
#endif

  client.print(POST_REQUEST);

  delay(100);
  
  String respChar;
  String response = "";
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    respChar = client.readStringUntil('\r');
    response = response + respChar ;
  }

  delay(100);

  #ifdef DBG_PRINT
  Serial.println("SERVER RESPONSE: \n" + response);
  #endif

  int body_start_idx = response.indexOf("[");
  int body_end_idx = response.lastIndexOf("]");

  response = response.substring( body_start_idx+1,body_end_idx );

  #ifdef DBG_PRINT
  Serial.println("BODY RESPONSE: \n" + response);
  #endif
  
  char decoded[256];
  int decodelen = base64_decode(decoded, (char *)response.c_str(), response.length() ) ;

  #ifdef DBG_PRINT
  Serial.println("Base64 decoded bytes : ");
  for(int i=0;i<decodelen;i++)
    Serial.print( String( decoded[i], HEX) + " "  ) ;
  Serial.println("\n");
  #endif
  
  //String decodeString = String(decoded);
  #ifdef DBG_PRINT
  //Serial.println("Base64 decode : " + decodeString );
  #endif
  
  //AES_Decrypt.process((uint8_t *)decodeString.c_str(), buf, decodeString.length() );
  AES_Decrypt.process((uint8_t *)decoded, buf, decodelen);
  //String decryptedString  = uintBufToString( buf , decodelen.length() );
  String decryptedString  = uintBufToString( buf , decodelen );
  #ifdef DBG_PRINT
  Serial.println("Decrypted string = "  + decryptedString );
  #endif
  
  String jsonResponse = decryptedString.substring( decryptedString.indexOf("{"), decryptedString.lastIndexOf("}")+1 );
  
  Serial.println("JSON RESPONSE : " + jsonResponse);
  
  JsonObject & root = jsonBuffer.parseObject(jsonResponse);

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
   if (e == 0) {
        updateSleepTime(p);
   }
   else {
    updateSleepTime(SLEEP_TIME_IF_ERROR);
   }
   return e;
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

void setup()
{
  pinMode(WAKE_EVENT_TYPE_PIN, INPUT_PULLUP);

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
