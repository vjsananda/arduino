/*

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

#define ZEROCROSS_FALL 2  //zero cross detect
#define ZEROCROSS_RISE  3
#define TRIAC 4 //triac gate
#define PULSE 4   //trigger pulse width (counts)

//Range 483
//Practical range seems to be 40 to 400
//Less than 40 bottom wave gets cut off.
int dim=200;

void setup()
{
  pinMode(TRIAC, OUTPUT);// Set AC Load pin as output
  pinMode(ZEROCROSS_RISE,INPUT);
  pinMode(ZEROCROSS_FALL,INPUT);

  // set up Timer1 
  //(see ATMEGA 328 data sheet pg 134
  OCR1A = 100;      // initialize the comparator  compared with TCNT1
  TIMSK1 = 0x03;    // enable comparator A and overflow interrupts  
            // 0x03=0b11 =OCIE1A and TOIE1
            // OCIE1A Timer/Counter1 Output Compare A Match interrupt enable. Interrupt set
            // TOIE1 Timer/Counter1 Overflow interrupt is enabled.
  TCCR1A = 0x00;    // timer control registers set for
  TCCR1B = 0x00;    // normal operation, timer disabled
  
  //This will fire ther interrupt at 120 Hz
  //INT0 is pin 2
  attachInterrupt(0, zeroCrossingInterrupt, RISING);  // Choose the zero cross interrupt # from the table above
  //INT 1 is pin 3
  attachInterrupt(1, zeroCrossingInterrupt, FALLING);  // Choose the zero cross interrupt # from the table above
}

void zeroCrossingInterrupt(){ //zero cross detect   
  TCCR1B=0x04; //start timer with divide by 256 input 16us tics  4=0b100 =CS12:1, CS11:0, CS10:0 table 16.5
  TCNT1 = 0;   //reset timer - count from zero
}

ISR(TIMER1_COMPA_vect){     // comparator match. overflow werd bereikt als de waarde van 'i'  wordt gematched
  digitalWrite(TRIAC,HIGH);   // Triac will be set high
  TCNT1 = 65536-PULSE;        // trigger pulse width telt 4 periodes van 16uS
}

ISR(TIMER1_OVF_vect){       // timer1 overflow wordt bereikt na 4x16us
  digitalWrite(TRIAC,LOW);    // turn off triac gate
  TCCR1B = 0x00;              // disable timer stopd unintended triggers
}

void loop()  {
  OCR1A = dim ;
  /*
  for (int i=5; i <= 128; i++){
    dimming=i;
    delay(10);
   }
   */
}

