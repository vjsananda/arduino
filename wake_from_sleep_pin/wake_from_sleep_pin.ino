#include <avr/sleep.h>

const byte LED = 13;
  
void wake ()
{
  // cancel sleep as a precaution
  sleep_disable();
  // precautionary while we do other stuff
  detachInterrupt (0);
}  // end of wake

void setup () 
  {
  digitalWrite (2, HIGH);  // enable pull-up
  Serial.begin(9600);
  Serial.println("setup done");
  }  // end of setup

void loop () 
{
 
  pinMode (LED, OUTPUT);
  for(int i=0;i<4;i++) {
    Serial.println("Interrupt triggered");
    digitalWrite (LED, HIGH);
    delay (50);
    digitalWrite (LED, LOW);
    delay (50);
  }
  pinMode (LED, INPUT);
  
  // disable ADC
  ADCSRA = 0;  
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();

  // Do not interrupt before we go to sleep, or the
  // ISR will detach interrupts and we won't wake.
  noInterrupts ();
  
  // will be called when pin D2 goes low  
  attachInterrupt (0, wake, FALLING);
  EIFR = bit (INTF0);  // clear flag for interrupt 0
 
  // turn off brown-out enable in software
  // BODS must be set to one and BODSE must be set to zero within four clock cycles
  MCUCR = bit (BODS) | bit (BODSE);
  // The BODS bit is automatically cleared after three clock cycles
  MCUCR = bit (BODS); 
  
  // We are guaranteed that the sleep_cpu call will be done
  // as the processor executes the next instruction after
  // interrupts are turned on.
  interrupts ();  // one cycle
  sleep_cpu ();   // one cycle

  } // end of loop
