//#include "ESP8266orig.h"
#include "ESP8266.h"
#include "wifi_credentials.h"

#define SSID        WIFI_SSID
#define PASSWORD    WIFI_PASSWORD

#define HOST_NAME   "www.google.com"
#define HOST_PORT    80

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(10,11); // RX, TX

ESP8266 wifi(EspSerial);

void setup(void)
{
    Serial.begin(9600);
    Serial.print("setup begin\r\n");

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
 
void loop(void)
{
    uint8_t buffer[1024] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }


    //char *hello = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n";
    char *hello = "GET / HTTP/1.0\r\n\r\n";
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
    
    //while(1);
    
}
