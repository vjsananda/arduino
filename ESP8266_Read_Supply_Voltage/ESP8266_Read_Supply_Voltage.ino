#include <ESP8266.h>
#include <SoftwareSerial.h>

//extern "C" {
 // uint16 readvdd33(void);
//}

void setup() { 
  WiFi.mode(WIFI_AP_STA); // readvdd33 doesn't work currently in WIFI_STA mode
  Serial.begin(9600); 
  Serial.println(); 
}

void loop() {
  float vdd = readvdd33() / 1000.0;
  Serial.print("Vdd: ");
  Serial.println(vdd);
  delay(1000);
}
