#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

void initTimer0() {
  TCCR0A |= (1 << COM0A0);             // toggle OC0A output on compare match
  TCCR0A |= (1 << WGM01);              // clear counter on compare match
  TCCR0B |= (1 << CS02) | (1 << CS00); //clock prescaler 1024
  OCR0A = 244;
}
void setup() {
  // put your setup code here, to run once:
  initTimer0();
  DDRB |= (1 << PB0); //DDR is a data direction register
}

void loop() {
  // put your main code here, to run repeatedly:

}
