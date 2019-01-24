#include <avr/sleep.h>
#include "ESP8266.h"
#include "wifi_credentials.h"

#define SSID        WIFI_SSID
#define PASSWORD    WIFI_PASSWORD

#define HOST_NAME   "api.thingspeak.com"
#define HOST_PORT    80
#define TIME_BETWEEN_TEXT_MESSAGES 10 //in seconds

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(10,11); // RX, TX

ESP8266 wifi(EspSerial);

const byte ch_pd = 8; //arduino pin number ESP8266 ch_pd is connected to
const byte LED = 13;

void pins_init()
{
  pinMode(ch_pd,OUTPUT);
  pinMode(LED, OUTPUT);
}

void wake ()
{
  // cancel sleep as a precaution
  sleep_disable();
  // precautionary while we do other stuff
  detachInterrupt (0);
}  // end of wake

void WifiPowerUp() {
  Serial.println("** Power Up Wifi **");
  digitalWrite(ch_pd,HIGH);
}

void WifiPowerDown() {
  Serial.println("-- Power Down Wifi --");
  digitalWrite(ch_pd,LOW);
}

void wifiSetup() {
    WifiPowerUp();

    delay(10);
    EspSerial.begin(9600);
    delay(10);
    
    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());

    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");

        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }
    
    Serial.print("setup end\r\n");
    WifiPowerDown();

//    initializeFilters();
  
    //send_text_message("test");
}

void send_text_message(String msg) {
   
   WifiPowerUp();
   delay(2000);

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    String preamble = "GET /apps/thinghttp/send_request?api_key=YNUF28WWU6S33RZH&number=5126570021&message=";
    String postamble = " HTTP/1.0\r\n\r\n";
    String GET_REQUEST = preamble + msg + postamble;

    Serial.print(GET_REQUEST);
    Serial.println("");
    Serial.print("REQUEST length = ");
    Serial.println(GET_REQUEST.length());
    
    wifi.send((const uint8_t*)GET_REQUEST.c_str(), GET_REQUEST.length());
    
    //delay(5000);
    
    String data = wifi.recvString(5000);
    if (data.length() > 0) {
        Serial.print("Received:[");
        Serial.println(data);
    }
    
    WifiPowerDown();

    delay(TIME_BETWEEN_TEXT_MESSAGES*1000);
    
    //initializeFilters();
  
}

void setup () 
  {
  digitalWrite (2, HIGH);  // enable pull-up
  Serial.begin(9600);
  pins_init();
  WifiPowerUp();
  wifiSetup();
  Serial.println("setup done");
  }  // end of setup

void loop () 
{
//  Serial.println("Begin loop");
  
//  send_text_message("Interrupt fired");

  } // end of loop
