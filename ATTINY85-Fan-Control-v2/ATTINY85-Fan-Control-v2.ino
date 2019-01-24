/*
 Version 3: Deployed 10/7/2018
 Fixed OCR1A values for high , medium and low speeds.
 Still some flakiness, not as fine gained control you would expect
 with 8 values.
 PULSE width less than 2 causes problems.

 Programming settings:
  * Board: ATTINY 25/45/85
  * Processor: ATTINY85
  * Clock: 1 MHz
  *
Rewrite for ATTINY85 Timers vjs 9/24/2018
Will use Timer1, an 8 bit timer, but with a wider presacler range
ATTINY running at 1MHz.

AC Voltage dimmer with Zero cross detection
Author: Charith Fernanado <a href="http://www.inmojo.com">  http://www.inmojo.com

</a>
Adapted by DIY_bloke
License: Creative Commons Attribution Share-Alike 3.0 License.
Attach the Zero cross pin of the module to Arduino External Interrupt pin
Select the correct Interrupt # from the below table 
(the Pin numbers are digital pins, NOT physical pins: 
digital pin 2 [INT0]=physical pin 4 and digital pin 3 [INT1]= physical pin 5)
check: <a href="http://arduino.cc/en/Reference/attachInterrupt">  http://www.inmojo.com

</a>

Pin    |  Interrrupt # | Arduino Platform
---------------------------------------
2      |  0            |  All -But it is INT1 on the Leonardo
3      |  1            |  All -But it is INT0 on the Leonardo
18     |  5            |  Arduino Mega Only
19     |  4            |  Arduino Mega Only
20     |  3            |  Arduino Mega Only
21     |  2            |  Arduino Mega Only
0      |  0            |  Leonardo
1      |  3            |  Leonardo
7      |  4            |  Leonardo
The Arduino Due has no standard interrupt pins as an iterrupt can be attached to almosty any pin. 

In the program pin 2 is chosen
*/ 

#include <avr/io.h>
#include <avr/interrupt.h>

//--------- PINS -----------------------------
#define ZEROCROSS  0  //zero cross detect

#define SPEED_CONTROL_BIT0  3 // Speed control input
#define SPEED_CONTROL_BIT1  4
#define SPEED_CONTROL_BIT2  1

#define TRIAC 2 //triac gate
//--------------------------------------------

#define PULSE 2   //trigger pulse width (counts)

//Range from 1 to 100
//The higher the number, more of the wave is chopped

int speed_control_bit0 ;
int speed_control_bit1 ;
int speed_control_bit2 ;
int speed_control ;

void setup()
{
  pinMode(TRIAC,OUTPUT);// Set AC Load pin as output
  
  pinMode(ZEROCROSS,INPUT);
  
  pinMode(SPEED_CONTROL_BIT0,INPUT_PULLUP);
  pinMode(SPEED_CONTROL_BIT1,INPUT_PULLUP);
  pinMode(SPEED_CONTROL_BIT2,INPUT_PULLUP);

  //Enable Pin Change Interrupts
  GIMSK = 0b00100000 ; //turn on and Pin change interrupts 
    
  //Use Arduino pin 0 as Pin change intrs
  PCMSK |= 0b00000001 ;
    
  // set up Timer1 compare value
  OCR1A = 10;      // initialize the comparator  compared with TCNT1

  // enable comparator A and overflow interrupts 
  //TIMSK != ( 1 << OCIE1A); //enable comparator interrupt
  //TIMSK != ( 1 << TOIE1) ; //enable overflow interrupt
  TIMSK = 0x44 ;

  //Timer 1, init, don't start
  TCCR1 = 0; 

  sei();
}

ISR(PCINT0_vect) {
  //Start timer with divide by 64 for a 1 MHz clock.
  //TCCR1 != (1 << CS12);
  //TCCR1 != (1 << CS11);
  //TCCR1 != (1 << CS10); 
  TCCR1 = 0x07; //The above is same as 0x0E
  
  TCNT1 = 0;   //reset timer - count from zero

  sei();
}

ISR(TIMER1_COMPA_vect){     // comparator match. overflow was achieved when the value of 'i' is matched
  digitalWrite(TRIAC,HIGH);   // Triac will be set high
  TCNT1 = 255-PULSE;        // trigger pulse width counts 4 periods of 16uS

  sei();
}

ISR(TIMER1_OVF_vect){       // timer1 overflow is reached after 4x16us
  digitalWrite(TRIAC,LOW);    // turn off triac gate
  TCCR1 = 0x00;              // disable timer stopd unintended triggers

  sei();
}

void loop()  {
  
  speed_control_bit0 = digitalRead(SPEED_CONTROL_BIT0);
  speed_control_bit1 = digitalRead(SPEED_CONTROL_BIT1);
  speed_control_bit2 = digitalRead(SPEED_CONTROL_BIT2);

  speed_control = (speed_control_bit2 << 2) | (speed_control_bit1 << 1) | speed_control_bit0;

  switch(speed_control) {
    case 7:
    OCR1A = 20 ;
    break ;
    
    case 6:
    OCR1A = 40 ;
    break;
    
    case 5:
    OCR1A = 60 ;
    break;

    case 4:
    OCR1A = 80 ;
    break;

    case 3:
    OCR1A = 100 ;
    break ;
    
    case 2:
    OCR1A = 120 ;
    break ;
    
    case 1:
    OCR1A = 130 ;
    break ;
    
    case 0:
    OCR1A = 150 ;
    break ;
    
  }
  
  delay(50);

}

