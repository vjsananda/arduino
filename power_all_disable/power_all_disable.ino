#include <avr/sleep.h>
#include <avr/power.h>

void setup () 
{

  // disable ADC
  ADCSRA = 0;  

  // turn off various modules
  power_all_disable ();
  
  set_sleep_mode (SLEEP_MODE_IDLE);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
 
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();              // sleep within 3 clock cycles of above    
}  // end of setup

void loop () { }
