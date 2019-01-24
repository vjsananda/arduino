/* ESP8266 Alarm prototype
 *  Works in conjunction with ATMEGA328 standalone for event detection
 *  atmega-interrupt-router sketch.
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
#include <ESP8266WiFi.h>
#include "wifi_credentials.h"

// WiFi settings
const char* ssid = WIFI_SSID ;
const char* password = WIFI_PASSWORD ;

#define WAKE_EVENT_TYPE_PIN 14

// Time to sleep (in milliseconds):
const int sleepTimeMS = 15000;

// Time to sleep (in seconds):
const int sleepTimeS = 10;

// Host
const char* host = "dweet.io";
volatile byte wake_event_type = 0;

void alarm_wakeup() {
  
}

void setup() 
{
  pinMode(WAKE_EVENT_TYPE_PIN,INPUT_PULLUP);

  //attachInterrupt( WAKE_EVENT_TYPE_PIN , alarm_wakeup, LOW);
  wake_event_type = digitalRead(WAKE_EVENT_TYPE_PIN);
   
  // Serial
  Serial.begin(115200);
  Serial.println("ESP8266-Standalone in normal mode");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Print the IP address
  Serial.println(WiFi.localIP());
  Serial.println("ESP8266-Standalone in normal mode");
  
  // Logging data to cloud
  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  if (wake_event_type) {
    
  // This will send the request to the server
  client.print(String("GET /dweet/for/mkz?message=ALARM") + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  }
  else {
    
      client.print(String("GET /dweet/for/mkz?message=PING") + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  }
  
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");

  // Sleep
  Serial.println("ESP8266 in sleep mode");

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
