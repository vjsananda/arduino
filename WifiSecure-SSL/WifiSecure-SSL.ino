// Simple Web client example
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "wifi_credentials.h"

char ssid[] = WIFI_SSID;           // your network SSID (name)
char pass[] = WIFI_PASSWORD ;           // your network password (use for WPA, or use as key for WEP)

//char server[] = "www.google.co.uk"; // name address for Google (using DNS)
char server[] = "mkzense.com"; 

int port = 443;
int ctr = 0;

WiFiClientSecure client;
//WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ctr++;
    Serial.print(".");
    if (ctr ==30) {
      ESP.deepSleep(2*1e6,WAKE_RFCAL); 
    }
  }
  
  Serial.println("Connected to wifi");
  Serial.println("\nStarting connection to server...");
  if (client.connect(server, port)) {
    //client.println("GET /search?q=how+far+to+the+moon& HTTP/1.1");    // Make a HTTP request:
    client.println("GET / HTTP/1.1");    // Make a HTTP request:
    //client.println("Host:  www.google.co.uk");
    client.println("Host:  mkzense.com");
    client.println("Connection: close");
    client.println();
  }
}

void loop() {
  while (client.available()) {// if there are incoming bytes available read and print them:
    char c = client.read();
    Serial.write(c);
    if (c=='\n' || c=='\r') Serial.println();
  }
}


