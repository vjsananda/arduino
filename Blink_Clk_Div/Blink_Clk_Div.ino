/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */
#include <avr/power.h>
#include <avr/sleep.h>

int delay_interval=256;




// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  clock_prescale_set(clock_div_256);
  ADCSRA=0;
  pinMode(13, OUTPUT);
  delay_interval = delay_interval/256 ;
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(delay_interval);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(delay_interval); 
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(delay_interval);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(delay_interval*5); // wait for a second
}
