//#include "ESP8266orig.h"
#include "ESP8266.h"
#include "wifi_credentials.h"

#define SSID        WIFI_SSID
#define PASSWORD    WIFI_PASSWORD

#define HOST_NAME   "api.thingspeak.com"
//#define HOST_NAME   "ACf695d769bad89b3bc0d66071ddc1b188:e3f0f41b42cd095660f37158669cd54b@api.twilio.com"
#define HOST_PORT    80

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(10,11); // RX, TX

ESP8266 wifi(EspSerial);

int ch_pd = 4;

void setup(void)
{
    pinMode(ch_pd,OUTPUT);
    
    Serial.begin(9600);
    Serial.print("setup begin\r\n");

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
}

void WifiPowerUp() {
  Serial.println("** Power Up Wifi **");
  digitalWrite(ch_pd,HIGH);
}

void WifiPowerDown() {
  Serial.println("-- Power Down Wifi --");
  digitalWrite(ch_pd,LOW);
}

void loop(void)
{
    uint8_t buffer[1024] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }


    //char *hello = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n";
    //char *hello = "GET / HTTP/1.0\r\n\r\n";
    char *hello = "GET /apps/thinghttp/send_request?api_key=YNUF28WWU6S33RZH&number=5126570021&message=hello_world_from_arduino_nano HTTP/1.0\r\n\r\n";
    //char *hello = "POST /2010-04-01/Accounts/ACf695d769bad89b3bc0d66071ddc1b188/Messages.json\r\nTo=5126570021\r\nFrom=+15126237477\r\nBody=this is a test HTTP/1.0\r\n\r\n";
    wifi.send((const uint8_t*)hello, strlen(hello));

    //delay(5000);
    
    String data = wifi.recvString(5000);
    if (data.length() > 0) {
        Serial.print("Received:[");
        Serial.println(data);
    }

    /*
     
    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }
    
     
    if (wifi.releaseTCP()) {
        Serial.print("release tcp ok\r\n");
    } else {
        Serial.print("release tcp err\r\n");
    }
     */

    WifiPowerDown();
    while(1);
    
}
