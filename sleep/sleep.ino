#include <avr/sleep.h>

void setup() {
  // put your setup code here, to run once:
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  sleep_cpu();
}

void loop() {
  // put your main code here, to run repeatedly:

}
