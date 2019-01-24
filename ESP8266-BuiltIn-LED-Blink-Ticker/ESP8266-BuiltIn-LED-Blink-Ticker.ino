/*
 * ESP8266 Built in LED is on on pin 1 or GPIO1
 * for newer boards
 * 
 * On some old boards it is pin 2
 * 
 * LED_BUILTIN is 1
 */
#include <ESP8266WiFi.h>
#include <Ticker.h>
#define LED_PIN LED_BUILTIN
//#define LED_PIN 1
//

Ticker flipper;
int count = 0;

void flip() {
  count++;
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));  

  //Stop blinking after 100 times
  if (count > 10) {
    flipper.detach();
    digitalWrite(LED_PIN,0);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("***********");
  // put your setup code here, to run once:
  pinMode(LED_PIN,OUTPUT);
  pinMode(12,INPUT_PULLUP);
  
  flipper.attach(0.5,flip);
  Serial.print("LED PIN : ");
  Serial.println(LED_PIN);
  Serial.print("LED BUILTIN :");
  Serial.println(LED_BUILTIN);

  delay(1000);

}

void loop() {
  if ( digitalRead(12) == 0 ) {
    count = 0;
    flipper.attach(0.2,flip);
  }
    Serial.print("LED PIN : ");
  Serial.println(LED_PIN);
  Serial.print("LED BUILTIN :");
  Serial.println(LED_BUILTIN);
  delay(1000);
}
