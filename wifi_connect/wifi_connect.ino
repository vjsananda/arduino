
//Wifi connect
//===================================================================
//Serial is the UART connected to the USB-serial port of the host PC.
//it is not connected to pins 0 and 1 of the Galileo board
//
//Serial1 is the UART connected to pins 0 and 1 of the board.
//These pins are connected to the RX & TX pins of the ESP8266 board
//
//Baud rates higher than 9600 don't work on this version of the
//ESP8266 firmware.
//The Serial monitor baud rate can be set as high as possible.
//
//Works: 6/23/2015 on both Mac and Windows.
//===================================================================
#include "wifi_credentials.h"

int ch_pd = 12 ;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600); // your esp's baud rate might be different
  pinMode(ch_pd,OUTPUT);
  digitalWrite(ch_pd,HIGH);

  reset();
  
  getFirmwareVersion();
  
  //setupWifi();
  getIPAddress();
  setupTCP();
  sendCmd();
}

void reset() {
  Serial.println("Start reset..");
  Serial1.println("AT+RST");
  delay(1000);
  Serial.println("Finish reset");
}
void getFirmwareVersion() {
  Serial.println("Start getFirmwareVersion");
  delay(300);
  Serial1.println("AT+GMR");
  delay(500);
  Serial1.println("AT+CWMODE=1");
  delay(1000);
  Serial.println("End getFirmwareVersion");
}

void getIPAddress() {
  delay(4000);
  Serial1.println("AT+CIFSR");
}
void setupWifi() {
  delay(250);
  Serial1.println("AT+CWMODE=1");
  delay(250);
  
  String WifiSetup = "AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PASSWORD + "\"" ;
  //Serial1.println(WifiSetup);
  //delay(4000);
}

void setupTCP() {
  delay(1000);
  //Setup multiple connections
  //Serial1.println("AT+CIPMUX=1");

  delay(4000);
  Serial1.println("AT+CIPSTART=\"TCP\",\"www.google.com\",80");
  delay(4000);
  
}

void sendCmd() {
  //String cmd="POST /apps/thinghttp/send_request?api_key=YNUF28WWU6S33RZH&number=+15126570021&message=arduino_text_message HTTP/1.0\r\n\r\n";
  String cmd = "GET / HTTP/1.0\r\nHost: www.google.com\r\n";
  Serial1.print("AT+CIPSEND=");
  Serial1.println(cmd.length());

  delay(2000);
  Serial1.println(cmd);
}

void loop()
{
  
  while(Serial1.available())
   {
     // The esp has data so display its output to the serial window
     Serial.write(Serial1.read());
   }


  if(Serial.available())
  {
    // the following delay is required because otherwise the arduino will read the first letter of the command but not the rest
    // In other words without the delay if you use AT+RST, for example, the Arduino will read the letter A send it, then read the rest and send it
    // but we want to send everything at the same time.
    //delay(1000);

    String command="";

    while(Serial.available()) // read the command character by character
    {
        // read one character
      command+=(char)Serial.read();
    }
    Serial1.println(command); // send the read character to the Serial1
  }
}
