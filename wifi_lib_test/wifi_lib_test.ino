#include <avr/sleep.h>
#include "ESP8266.h"
#include <avr/wdt.h>
#include <avr/power.h>

#include "wifi_credentials.h"

#define SSID        WIFI_SSID
#define PASSWORD    WIFI_PASSWORD

//#define HOST_NAME   "api.thingspeak.com"
//#define HOST_PORT    80

#define HOST_NAME   "192.168.1.19"
#define HOST_PORT    3000

#define TIME_BETWEEN_TEXT_MESSAGES 2 //in seconds
#define CLK_DIV 1
#define DBG_PRINT 1
#define PING_INTERVAL 1440 //in watchdog intervals, max watchdog time = 8 sec

#include "SoftwareSerial.h"
SoftwareSerial EspSerial(10,11); // RX, TX

ESP8266 wifi(EspSerial, Serial);

const byte ch_pd = 8; //arduino pin number ESP8266 ch_pd is connected to
const byte LED = 13;

void pins_init()
{
  pinMode(ch_pd,OUTPUT);
  pinMode(LED, OUTPUT);
}

void WifiPowerUp() {
  #ifdef DBG_PRINT
  Serial.println(F("** Power Up Wifi **"));
  #endif
  
  digitalWrite(ch_pd,HIGH);
}

void WifiPowerDown() {
  #ifdef DBG_PRINT
  Serial.println(F("-- Power Down Wifi --"));
  #endif
  digitalWrite(ch_pd,LOW);
}

void wifiSetup() {
    WifiPowerUp();

    delay(10/CLK_DIV);
    EspSerial.begin(9600*CLK_DIV);
    delay(10/CLK_DIV);

    #ifdef DBG_PRINT
    Serial.print(F("FW Version:"));
    Serial.println(wifi.getVersion().c_str());
    #endif
    
    if (wifi.setOprToStationSoftAP()) {
        #ifdef DBG_PRINT
        Serial.print(F("to station + softap ok\r\n"));
        #endif
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("to station + softap err\r\n"));
        #endif
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        #ifdef DBG_PRINT
        Serial.print(F("Join AP success\r\n"));

        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());  
        #endif     
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("Join AP failure\r\n"));
        #endif
    }
    
    if (wifi.disableMUX()) {
        #ifdef DBG_PRINT
        Serial.print(F("single ok\r\n"));
        #endif
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("single err\r\n"));
        #endif
    }

    #ifdef DBG_PRINT
    Serial.print(F("setup end\r\n"));
    #endif
    
    //WifiPowerDown();

}

void send_text_message(String msg) {
   
   WifiPowerUp();
   delay(2000/CLK_DIV);

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        #ifdef DBG_PRINT
        Serial.print(F("create tcp ok\r\n"));
        #endif
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("create tcp err\r\n"));
        #endif
    }

    String preamble = "GET /apps/thinghttp/send_request?api_key=YNUF28WWU6S33RZH&number=5126570021&message=";
    String postamble = " HTTP/1.0\r\n\r\n";
    String GET_REQUEST = preamble + msg + postamble;

    #ifdef DBG_PRINT
    Serial.print(GET_REQUEST);
    Serial.println("");
    Serial.print(F("REQUEST length = "));
    Serial.println(GET_REQUEST.length());
    #endif
    
    wifi.send((const uint8_t*)GET_REQUEST.c_str(), GET_REQUEST.length());
    
    //delay(5000);
    
    String data = wifi.recvString(5000);
    if (data.length() > 0) {
        #ifdef DBG_PRINT
        Serial.print(F("Received:["));
        Serial.println(data);
        #endif
    }

    
    WifiPowerDown();

    delay(TIME_BETWEEN_TEXT_MESSAGES*1000/CLK_DIV);
  
}

void create_tcp_conn() {

     while(1) {
      if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
         #ifdef DBG_PRINT
         Serial.print(F("create tcp ok\r\n"));
         #endif
         break ;
      } else {
         #ifdef DBG_PRINT
         Serial.print(F("create tcp err\r\n"));
         #endif
      }
      delay(1000);
     }
}

void test_post_request(String msg) {
   
   WifiPowerUp();
   delay(2000/CLK_DIV);

    create_tcp_conn();

    String preamble = "POST /postform HTTP/1.0\r\n";
    String body = "param1=123&param2=456\r\n";
    String headers = "content-type: application/x-www-form-urlencoded\r\ncontent-length:21\r\n" ;
    String blank_line = "\r\n";

    String POST_REQUEST = preamble + headers + blank_line + body + blank_line  ;

    #ifdef DBG_PRINT
    Serial.print(POST_REQUEST);
    Serial.println("");
    Serial.print("REQUEST length = ");
    Serial.println(POST_REQUEST.length());
    #endif
    
    wifi.send((const uint8_t*)POST_REQUEST.c_str(), POST_REQUEST.length());
    
    //delay(5000);

    while(1) {
     String data = wifi.recvString(5000);
     
     if (EspSerial.overflow())
       Serial.println(F("Software-Serial Rx buffer overflowed"));
       
     if (data.length() > 0) {
         #ifdef DBG_PRINT
         Serial.println("Received:");
         Serial.println(data);
         #endif
     }
     else {
      break ;
     }
     Serial.print("+");
    }
    Serial.println("*");
    
   
    WifiPowerDown();

    //delay(TIME_BETWEEN_TEXT_MESSAGES*1000/CLK_DIV);
    
    //initializeFilters();
  
}

void setup () 
  {
  digitalWrite (2, HIGH);  // enable pull-up
  #ifdef DBG_PRINT
  Serial.begin(9600*CLK_DIV);
  #endif
  
  clock_prescale_set(clock_div_1);
  pins_init();
  
  delay(1000);
  wifiSetup();

  //WifiPowerUp();

  Serial.println(wifi.getLocalIP() );

  #ifdef DBG_PRINT
  Serial.println(F("setup done"));
  #endif
  
  }  // end of setup

void loop () 
{
  while(1)
    delay(1000);
    
} // end of loop
