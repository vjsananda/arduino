//#define LED_PIN LED_BUILTIN
#define LED_PIN 2
/*
 * LED_BUILTIN is 1, but for some ESP8266 boards the built in LED is on pin 2
 */
 
#include <ESP8266WiFi.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_PIN,OUTPUT);

  delay(2000);
  
  Serial.print("LED PIN : ");
  Serial.println(LED_PIN);
  Serial.print("LED BUILTIN :");
  Serial.println(LED_BUILTIN);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_PIN, LOW);  
  delay(200);
  digitalWrite(LED_PIN, HIGH);  
  delay(200);

}
