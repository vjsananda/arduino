ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
   count--;
}  // end of WDT_vect

void wake ()
{
  // cancel sleep as a precaution
  sleep_disable();
  // precautionary while we do other stuff
  detachInterrupt (0);
  alarm_detect = 1;
  
}  // end of wake

