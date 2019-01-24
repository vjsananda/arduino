/*
 Interrupt router to control events to ESP8266.
 Works in conjunction with ESP8266-Standalone-WifiTest
 
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

 Last update: 4/16/2017
*/

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#define ALARM_WAKE_PIN  3
#define  ESP_WAKE_PIN 4

#define ESP_RESET_PIN 8
#define ALARM_SOURCE_PIN 9

//#define ESP_WAKE_INT 0
#define ALARM_WAKE_INT 1

#define ESP_WAKE_EVENT 0
#define ALARM_WAKE_EVENT 1

volatile byte alarm_detect = 0;

void pins_init()
{
 pinMode(ESP_WAKE_PIN, INPUT_PULLUP);//if INPUT, floating values will cause repeated PIN CHANGE interrupts
 pinMode(ALARM_WAKE_PIN, INPUT_PULLUP);
 
 pinMode(ESP_RESET_PIN,OUTPUT);
 pinMode(ALARM_SOURCE_PIN, OUTPUT);
}

void esp_reset() {
  digitalWrite(ESP_RESET_PIN,LOW);
  delay(500);
  digitalWrite(ESP_RESET_PIN,HIGH);
}

void alarm_wake() {
  sleep_disable();
  detachInterrupt(ALARM_WAKE_INT);
  digitalWrite(ALARM_SOURCE_PIN, ALARM_WAKE_EVENT);
  alarm_detect=1;
}

//void esp_wake() {
ISR(PCINT2_vect) {
  sleep_disable();
  //cli();adding this code causes download fails
  digitalWrite(ALARM_SOURCE_PIN, ALARM_WAKE_EVENT);

  //Handle the case where the ALARM pin gets pulled low the exact time
  //of the ESP RTC ping. Since the ALARM interrupt is set to trigger
  //on the falling edge, this ALARM event might be missed.
  //Cannot have it trigger on LOW, because it will generate repeated
  //resets to the ESP.
  if ( digitalRead(ALARM_WAKE_PIN) == 0 ) {
    alarm_detect = 1;
  } else {
    alarm_detect = 0;
  }
  //sei();
}

void setup ()
  {
  //cli();//turn of interrupts,another way compared to noInterrupts()
  
  pins_init();
  esp_reset();

  PCICR |= 0b00000100; //turn on port d (PCINT16 to PCINT23)
  PCMSK2 |= 0b00010000; //enable PCINT20
  
  Serial.begin(115200);
  Serial.println("setup done");
  
  //sei();//turn on interrupts,another way
}  // end of setup

void loop ()
{
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();

  // Do not interrupt before we go to sleep, or the
  // ISR will detach interrupts and we won't wake.
  noInterrupts();
  
  // will be called when pin  goes low  
  attachInterrupt (ALARM_WAKE_INT, alarm_wake, FALLING);

  // will be called when pin  goes high to low  
  //attachInterrupt (ESP_WAKE_INT, esp_wake, FALLING);
  
  //EIFR = bit (INTF0);  // clear flag for interrupt 0
  EIFR = bit (INTF1);  // clear flag for interrupt 1
 
  // turn off brown-out enable in software
  // BODS must be set to one and BODSE must be set to zero within four clock cycles
  MCUCR = bit (BODS) | bit (BODSE);
  // The BODS bit is automatically cleared after three clock cycles
  MCUCR = bit (BODS); 
  
  //Disable ADC
   ADCSRA = 0;
  
  // We are guaranteed that the sleep_cpu call will be done
  // as the processor executes the next instruction after
  // interrupts are turned on.
  interrupts ();  // one cycle
  sleep_cpu ();   // one cycle

    esp_reset();
    if ( alarm_detect ) {
      digitalWrite(ALARM_SOURCE_PIN, ALARM_WAKE_EVENT);
      Serial.println("ALARM event");
    }
    else {
      digitalWrite(ALARM_SOURCE_PIN, ESP_WAKE_EVENT);
      Serial.println("ESP RESET event");
    }

  delay(500);
} // end of loop
