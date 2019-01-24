// Library
#include <FS.h>

//Needed for Wifi manager
#include <DNSServer.h>
#include <WiFiManager.h>   

#include <ESP8266WiFi.h>


void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  ESP.deepSleep(5*1e6, WAKE_RFCAL);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("*** Woke from Sleep ***");
  delay(10000);

}
