// Library
#include <ESP8266WiFi.h>
#include "wifi_credentials.h"

// WiFi settings
const char* ssid = WIFI_SSID ;
const char* password = WIFI_PASSWORD ;

// Time to sleep (in seconds):
const int sleepTimeS = 10;

// Host
const char* host = "dweet.io";

void setup() 
{

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
  
  // This will send the request to the server
  client.print(String("GET /dweet/for/myesp8266?message=lowpower") + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
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
  ESP.deepSleep(sleepTimeS * 1000000);
  
}

void loop() 
{
  //Doesn't wake from deep sleep without reset
  Serial.println("Waking from sleep..");
  ESP.deepSleep(sleepTimeS * 1000000);
}
