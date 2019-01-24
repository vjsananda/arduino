/*
 PUBSUB MQTT library
 
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 
 It connects to an MQTT server then:
  - publishes to topic "sensorsiot/feeds/luminosity" every two seconds. Luminosity here is 
    just the VCC value which is published if it is different from the previous value.
  - To see these messages (from any machine with an mqtt client installed) run:
      mosquitto_sub -h mkzense.com -t "sensorsiot/feeds/luminosity"
      
  - subscribes to the topic "sensorsiot/feeds/command", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If message is "ON" turns LED on, if "OFF" turns it off.
      mosquitto_pub -h mkzense.com -t "sensorsiot/feeds/command" -m "ON" 
      mosquitto_pub -h mkzense.com -t "sensorsiot/feeds/command" -m "OFF" 

  - To see value of LED when it changes, subscribe to
    mosquitto_sub -h mkzense.com -t "sensorsiot/feeds/clientStatus" 
    (Note only changes will be published, there is no periodic polling for the value)
      
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
  Code from Andreas Spiess
*/

//Builtin LED for ESP8266 board (can't use LED_BUILTIN which is GPIO1)
//Builtin LED for ESP8266 board: ESP 12 : pin GPIO2
//LED for SONOFF is GPIO13

//Have to turn Serial off, because using RX and TX pins as GPIO

//SONOFF Pins
#define PIN_SONOFF_RELAY   12 //High to turn on
#define PIN_TX              1
#define PIN_RX              3
#define PIN_SONOFF_LED     13 //Low to turn on

#define PIN_SPEED_BIT0     PIN_RX
#define PIN_SPEED_BIT1     PIN_TX
#define PIN_SPEED_BIT2     14

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//define WIFI_SSID and WIFI_PASSWORD
//in this file
#include "wifi_credentials.h"

/*
 *  I save my credentials in a file to protect them. Please comment this line and fill out
 *  your credentials below
 *
 */
//#include "credentials_CloudMQTT.h"
//#include "credentials_Adafruit_IO.h"

#define ssid          WIFI_SSID
#define password      WIFI_PASSWORD

//MQTT server info
#define SERVER          "mkzense.com"
#define SERVERPORT      1883  //This is the default MQTT broker port
#define MQTT_USERNAME   ""
#define MQTT_KEY        ""
#define USERNAME          "sensorsiot/"
#define PREAMBLE          "feeds/"
//Topics
#define T_LUMINOSITY      "luminosity"
#define T_CLIENTSTATUS    "clientStatus"
#define T_COMMAND         "command"


unsigned long entry;
byte clientStatus, prevClientStatus = 99;
float luminosity, prevLumiosity = -1;
char valueStr[5];

WiFiClient WiFiClient;

// create MQTT object
PubSubClient client(WiFiClient);

int ctr = 0;

void setup() {

  delay(3000);

  pinMode(PIN_SONOFF_LED, OUTPUT);
  pinMode(PIN_SONOFF_RELAY, OUTPUT);

/*
  pinMode(PIN_SPEED_BIT0, FUNCTION_3);
  pinMode(PIN_SPEED_BIT0, OUTPUT);
  
  pinMode(PIN_SPEED_BIT1, FUNCTION_3);
  pinMode(PIN_SPEED_BIT1, OUTPUT);
*/

  pinMode(PIN_SPEED_BIT2, OUTPUT);

  //By default turn relay on
  //set all speed control bits to 1 (highest speed)
  digitalWrite(PIN_SONOFF_LED, LOW);
  digitalWrite(PIN_SONOFF_RELAY, 1);
  
  digitalWrite(PIN_SPEED_BIT0,1);
  digitalWrite(PIN_SPEED_BIT1,1);
  digitalWrite(PIN_SPEED_BIT2,1);
  
  //Serial.begin(115200);
  delay(100);
  //Serial.println();
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  delay(1000);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ctr++;
    //Serial.print(".");
    if (ctr ==30) {
      ESP.deepSleep(2*1e6,WAKE_RFCAL); 
    }
  }

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);

  client.setServer(SERVER, SERVERPORT);
  client.setCallback(callback);
}

void loop() {
  delay(100);
  //Serial.println("loop");
  yield();
  if (!client.connected()) {
    //Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("", MQTT_USERNAME, MQTT_KEY)) {
      //Serial.println("connected");
      // ... and resubscribe
      client.subscribe(USERNAME PREAMBLE T_COMMAND, 1);
      client.subscribe(USERNAME PREAMBLE "test", 1);
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

/*
  if (millis() - entry > 1200) {
    Serial.println("Measure");
    entry = millis();
    luminosity = ESP.getVcc();
  }

  if (client.connected() && prevLumiosity != luminosity) {
    Serial.println("Publish Luminosity");
    String hi = (String)luminosity;
    hi.toCharArray(valueStr, 5);
    client.publish(USERNAME PREAMBLE T_LUMINOSITY, valueStr);
    prevLumiosity = luminosity;
    delay(500);
  }
*/

  if (client.connected()&& prevClientStatus != clientStatus ) {
    //Serial.println("Publish Status");

    String hi = (String)clientStatus;
    hi.toCharArray(valueStr, 2);
    client.publish(USERNAME PREAMBLE T_CLIENTSTATUS, valueStr);
    prevClientStatus = clientStatus;
  }
  client.loop();
}

//----------------------------------------------------------------------

void callback(char* topic, byte * data, unsigned int length) {
  // handle message arrived {

  String command ;
  int isFanSpeedCommand ;
  int fanSpeedBit0,fanSpeedBit1,fanSpeedBit2;
  int fanSpeed = 7;
  
  char tmpCharArray[30];

  //Serial.print(topic);
  //Serial.print(": ");
  
  for (int i = 0; i < length; i++) {
    //Serial.print((char)data[i]);
    tmpCharArray[i] = (char)data[i] ;
  }
  tmpCharArray[length] = '\0';
  
  //Serial.println();

  command = String(tmpCharArray);
  //Serial.println(command);
  //Serial.println();

  //Is command of the format FAN_SPEED_0, FAN_SPEED_1 etc
  isFanSpeedCommand = command.substring(0,9) == "FAN_SPEED";

  if (isFanSpeedCommand) {
    fanSpeed = command.substring(10).toInt();
    fanSpeedBit0 = fanSpeed & 0x01;
    fanSpeedBit1 = (fanSpeed & 0x02) >> 1;
    fanSpeedBit2 = (fanSpeed & 0x04) >> 2;
    
    digitalWrite(PIN_SPEED_BIT0, fanSpeedBit0);
    digitalWrite(PIN_SPEED_BIT1, fanSpeedBit1);
    digitalWrite(PIN_SPEED_BIT2, fanSpeedBit2);
    
    //Serial.print("Fan Speed = ");
    //Serial.print(fanSpeedBit2);
    //Serial.print(fanSpeedBit1);
    //Serial.print(fanSpeedBit0);
    //Serial.println();
  }
  else
  if (command ==  "FAN_OFF") {    
     clientStatus = 0;
     digitalWrite(PIN_SONOFF_LED, HIGH );
     digitalWrite(PIN_SONOFF_RELAY, LOW);
     //Serial.println("Send command: FAN OFF");
  }
  else
  if ( command == "FAN_ON") {
      clientStatus = 1;
      digitalWrite(PIN_SONOFF_LED, LOW );
      digitalWrite(PIN_SONOFF_RELAY, HIGH);
      //Serial.println("Send command: FAN ON");
  }
 
  //Serial.println();
}


