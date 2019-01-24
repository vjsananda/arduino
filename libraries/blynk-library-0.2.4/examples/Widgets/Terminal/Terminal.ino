/**************************************************************
 * Blynk is a platform with iOS and Android apps to control
 * Arduino, Raspberry Pi and the likes over the Internet.
 * You can easily build graphic interfaces for all your
 * projects by simply dragging and dropping widgets.
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social networks:            http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 *
 * Blynk library is licensed under MIT license
 * This example code is in public domain.
 *
 **************************************************************
 * You can send/receive any data using WidgetTerminal object.
 *
 * App dashboard setup:
 *   Terminal widget on V1
 *
 **************************************************************/

#define BLYNK_PRINT Serial
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Attach virtual serial terminal to virtual pin 1
WidgetTerminal terminal(1);

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth);

  Blynk.connect();

  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.flush();
}

BLYNK_WRITE(1)
{
  BLYNK_LOG("Hey! I got a new message!");

  // Send it back
  terminal.println("I got:");
  terminal.write(param.getBuffer(), param.getLength());
  terminal.flush();
}

void loop()
{
  Blynk.run();
}

