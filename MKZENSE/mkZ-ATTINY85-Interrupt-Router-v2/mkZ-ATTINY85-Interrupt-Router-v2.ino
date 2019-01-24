/*
 * ATTINY - ATTINY85 version 2 11/11/2017
 Interrupt router to control events to ESP8266.

 Program at 1MHz

 Program using Arduino as ISP, and SCK, MOSI, MISO connections.
 Remove STATUS_LED , pin 3, When high indicates it is RTC wake up event.
 Need this because there are 3 sources of RESET to the ESP.
 Power-on, RTC and Sensor-Event.

 See:
 https://thewanderingengineer.com/2014/08/11/pin-change-interrupts-on-attiny85/

 For Pin change interrupt differences from an ATMEGA.

 ATTINY has only a single external interrupt (INT0).and a single Pin Change interrupt group.
 PCINT0_vect. Trouble using the external interrupt (INT0) on an ATTINY.
 So using 2  Pin Change Interrupts.

 >>------- Rest of this write up is from the ATMEGA version of the writeup.
 
 Works in conjunction with mkZ-Standalone-Alarm

 With power optimizations, main one being turning of ADC.
 Sleep power = 0.4 uA. With ADC on sleep power = 400 uA
 
 Purpose:
 When the ESP8266 is in deep sleep, the RTC inside can wake it up
 by sending a low pulse on GPIO16. This has to be wired to the RESET
 pin on the ESP8266, waking it up.

 The question is how do we detect external alarm events and have it 
 wake up an ESP8266 in deep sleep.

 This interrupt router sketch, uses an arduino to detect 2 signals going low.
 1 signal would be the GPIO16 pin from the ESP8266, this is the periodic RTC
 wake event.
 The 2nd signal is the actual ALARM event. 

 The ATMEGA328 has 2 external interrupts, and we could have used 1 for each
 of the above events. (INT0, INT1, arduino digital pins 2 and 3).
 However the goal is to use an ATTINY85, which only has 1 external interrupt.

 All pins however can be configured for PCINTR, Pin Change interrupts. So this
 sketch uses a PCINTR for the ALARM event. The ALARM event pin used is arduino
 digital pin 4 or PCINT20. The PCINTRs are classified into 3 groups (groups 0,1,2)
 , each with with 1 ISR. PCINT20 is part of group 2. Hence the PCMSK2 etc.
 Ref: http://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts

 Once an interrupt (either external or PCINTR) is detected, we set the ALARM_SOURCE_PIN
 high (if ALARM) or low (if periodic GPIO16 from ESP). 

 In choosing between PCINTR and external. External interrupts have more flexibility
 and ease of use. In the attachInterrupts() call, we can specify, rising, falling or low.
 THought of using LOW for the alarm event, since if the alarm event happens to occur at the 
 exact time of the ESP RTC wakeup, and we only trigger on FALLING,
 the alarm event will be missed. 
 Doesn't quite work in practice, because we will end up generating repeated resets to
 the ESP if the ALARM is level sensitive. So we use FALLING for the ALARM event, but
 we make sure we sample the ALARM_EVENT pin during the periodic ESP RTC wakeup events.

 The ESP will sample this pin and know the source of the reset coming in. That way
 it knows whether it is the periodic RTC waking it up, or an actual alarm event.

 Last update: 5/13/2017
*/

#include <avr/sleep.h>
#include <avr/interrupt.h>

//Duration of reset pulse
#define RESET_LOW_DURATION 1000

//How long ATTINY is active driving status pins.
#define STATUS_DURATION 10000

//These are the 2 interrupt pins
#define ESP_WAKE_PIN     0
#define ALARM_WAKE_PIN   1

#define ESP_WAKE_STATUS_PIN  3
#define ALARM_STATUS_PIN     4
#define ESP_RESET_PIN        2

volatile byte esp_wake ;
volatile byte alarm_wake ;
volatile byte do_reset ;

void esp_reset() {
  digitalWrite(ESP_RESET_PIN,LOW);
  delay(RESET_LOW_DURATION);
  digitalWrite(ESP_RESET_PIN,HIGH);
}

void setup() {

    pinMode(ESP_WAKE_STATUS_PIN,OUTPUT);
    pinMode(ESP_RESET_PIN,OUTPUT);
    
    pinMode(ESP_WAKE_PIN, INPUT);
    digitalWrite(ESP_WAKE_PIN, HIGH);
    
    pinMode(ALARM_WAKE_PIN,INPUT);
    digitalWrite(ALARM_WAKE_PIN,HIGH);
    
    pinMode(ALARM_STATUS_PIN, OUTPUT);
    
    //Enable Pin Change Interrupts
    GIMSK = 0b00100000 ; //turn on and Pin change interrupts 
    
    //Use Arduino pins 0 & 1 as PC intrs
    PCMSK |= 0b00000011 ;
    
    //Can't call this twice, will send out multiple alarms
    esp_reset();

} // setup

void sleep() {

    cli();
    
    ADCSRA &= ~_BV(ADEN);                   // ADC off
    
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

    sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  
    sei();                                  // Enable interrupts
    
    sleep_cpu();                            // sleep

    cli();  // Disable interrupts
        
    sleep_disable();                        // Clear SE bit
    
} // sleep

ISR(PCINT0_vect) {
    esp_wake = digitalRead(ESP_WAKE_PIN);
    alarm_wake = digitalRead(ALARM_WAKE_PIN);
}

void loop() {
    sleep();

    do_reset = 0;
    delay(500);

    if (esp_wake == 0) {
      digitalWrite(ESP_WAKE_STATUS_PIN,HIGH);
      digitalWrite(ALARM_STATUS_PIN,LOW);
      do_reset=1;
    }
    else {        
      //Reread alarm wake pin, it should still be low
      //This is to prevent an alarm being triggered when alarm pin
      //is being cleared, low to high
      alarm_wake = digitalRead(ALARM_WAKE_PIN);
  
      if (alarm_wake == 0) {
        digitalWrite(ALARM_STATUS_PIN, HIGH);
        digitalWrite(ESP_WAKE_STATUS_PIN,LOW);
        do_reset=1;
      }
    }

    if (do_reset) {
       esp_reset();
  
       //Time for ESP to sample wake and alarm status pins
       delay(STATUS_DURATION);
       
       digitalWrite(ESP_WAKE_STATUS_PIN,LOW);
       digitalWrite(ALARM_STATUS_PIN,LOW);
    }

    //Re-enable interrupts
    sei();
}

