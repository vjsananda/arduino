#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

const byte LED = 13;
byte count = 0;

void flash ()
  {
  pinMode (LED, OUTPUT);
  for (byte i = 0; i < 10; i++)
    {
    digitalWrite (LED, HIGH);
    delay (5);
    digitalWrite (LED, LOW);
    delay (5);
    }
    
  pinMode (LED, INPUT);
    
  }  // end of flash
  
// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
   count++;
   if (count == 500000) count = 0;
}  // end of WDT_vect
 
void setup () {
  //Serial.begin(38400);
  count = 0; 
  clock_prescale_set(clock_div_8);
  }

void loop () 
{

 //if (count == 4)
  //flash ();
  //delay(10);
  //Serial.println("Flash");
  //delay(10);
  
  // disable ADC
  ADCSRA = 0;  

  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 

  //WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);    // set WDIE, and 1 second delay
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  //WDTCSR = bit (WDIE) ;    // set WDIE, and 16 mill seconds delay

  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
 
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  
  
  // cancel sleep as a precaution
  sleep_disable();
  
  } // end of loop
