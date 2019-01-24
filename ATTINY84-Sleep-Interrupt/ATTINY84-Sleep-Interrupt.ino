/*
 * Multiple interrupts on ATTINY84
 * Had trouble using INT0 (external interrupt)
 * Wasn't able to wake up from sleep using INT0
 * 
 * Decided to use 2 Pin change interrupts, PCINT0 and PCINT1.
 * Read digital Value from both and that will tell us which one 
 * got pulled low.
 * 
 * VJS: 4/23/2017
 */

#include <avr/sleep.h>
#include <avr/interrupt.h>

const int Intr0_Pin                     = 0;
const int Intr1_Pin                     = 1;
const int statusLED                     = 9;

volatile byte pcint0 ;
volatile byte pcint1 ;

void flash(int _delay ) {
    // Flash quick sequence so we know setup has started
    for (int k = 0; k < 10; k = k + 1) {
        if (k % 2 == 0) {
            digitalWrite(statusLED, HIGH);
            }
        else {
            digitalWrite(statusLED, LOW);
            }
        delay(_delay);
     } // for
}

void setup() {

    pinMode(Intr0_Pin, INPUT);
    digitalWrite(Intr0_Pin, HIGH);
    pinMode(Intr1_Pin,INPUT);
    digitalWrite(Intr1_Pin,HIGH);
    
    //Enable Pin Change Interrupts
    GIMSK = 0b00010000 ; //turn on and Pin change interrupts PCINT0 to 7
    
    //Use Arduino pins 0 & 1 as PC intrs
    PCMSK0 |= 0b00000011 ;
    
    pinMode(statusLED, OUTPUT);
    flash(250);

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
    
    sei();                                  // Enable interrupts
} // sleep

ISR(PCINT0_vect) {
    pcint0 = digitalRead(0);
    pcint1 = digitalRead(1);
}

void loop() {
    sleep();

    if (pcint0 == 0)
      flash(100);

    if (pcint1 == 0)
      flash(250);

    delay(1000);
}
