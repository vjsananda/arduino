#include <avr/sleep.h>
#include <avr/power.h>

void setup() {
  // put your setup code here, to run once:
  clock_prescale_set(clock_div_256);
  ADCSRA=0;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();

}

void loop() {
  // put your main code here, to run repeatedly:

}
