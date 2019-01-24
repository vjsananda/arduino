/* =====================================================================
 *  Long Sleep example with guaranteed wakeup after any number of hours.
 *  ====================================================================
 *  The maximum deep sleep duration is 2**32 microseconds
 *  ( The call to ESP.deepSleep , argument is a 32 bit unsigned )
 *  This works out to only about 71 min
 *  For sleep periods longer than that, have to periodically wake up,
 *  decrement a counter in RTC memory (which is retained during deepSleep),
 *  and then when the counter reaches 0, wake up the entire chip, connect wifi 
 *  record sensor data etc (do the periodic work we want).
 *  So basically during the intermediate wake up from deep sleep, we don't want
 *  to do anything other than to update our RTC counter.
 *  For this reason we wake up with mode WAKE_RF_DISABLED for the lowest power
 *  consumption. The last deep sleep call will use WAKE_RF_DEFAULT or whatever we
 *  want to set.
 *  
 *  The sample code below uses a function updateSleepTime to autocompute the RTC 
 *  count value (dsCount) and the the deep sleep intervals. Because the total sleep
 *  duration desired may not be an exact multiple of the the deep sleep interval 
 *  (DEEP_SLEEP_THRESH), it also computes dsPingLastInt which is essentially the
 *  modulo reminder after dividing the TOTAL_SLEEP_DURATION by DEEP_SLEEP_THRESH.
 *  So the last time the ESP sleeps before the RTC count goes to 0, the ESP will
 *  sleep for dsPingLastInt. 
 *  We always add a 1, to make sure dsPingLastInt is not 0 (it could be if the
 *  TOTAL_SLEEP_DURATION is an exact multiple of DEEP_SLEEP_THRESH). Calling
 *  ESP.deepSleep with an argument of 0, will put it to sleep forever, which is
 *  not we want in this application.
 *  When dsCount reaches 0, we recompute and reload dsCount in RTC mem and the
 *  cycle continues.
 */

#include <ESP8266WiFi.h>

#define TOTAL_SLEEP_DURATION        75
#define DEEP_SLEEP_THRESH           22

// RTC-MEM Adresses
#define RTC_BASE 64
#define RTC_STATE 68
#define RTC_PING_COUNT 72
#define RTC_PING_INT  76
#define RTC_PING_LAST_INT  80

uint32_t  dsCount ;
uint32_t  dsPingInt ;
uint32_t  dsPingLastInt ;

void updateSleepTime(uint32_t _in) {
  uint32_t thresh = DEEP_SLEEP_THRESH ; 
  if ( _in > thresh ) {
    dsCount = _in / thresh + 1;
    dsPingInt = thresh ;
    dsPingLastInt = _in % thresh + 1 ;//cannot be 0
  }
  else {
    dsCount = 1;
    dsPingInt = _in  + 1; //cannot be 0
    dsPingLastInt = _in + 1;//cannot be 0
  }
  ESP.rtcUserMemoryWrite(RTC_PING_COUNT, &dsCount, sizeof(dsCount)); 
  ESP.rtcUserMemoryWrite(RTC_PING_INT, &dsPingInt, sizeof(dsPingInt)); 
  ESP.rtcUserMemoryWrite(RTC_PING_LAST_INT, &dsPingLastInt, sizeof(dsPingLastInt));
}

void setup() {
  
  RFMode mode ;
  uint32_t magicNumber ;
  uint32_t sleepTimeS ;
  
  Serial.begin(115200);
  Serial.println();

  //Detect cold start
  ESP.rtcUserMemoryRead(RTC_BASE, &magicNumber, sizeof(magicNumber)); 
  if ( magicNumber != 0xdeadbeef ) {
    Serial.println("**** COLD START ****");
    magicNumber = 0xdeadbeef;
    ESP.rtcUserMemoryWrite(RTC_BASE, &magicNumber, sizeof(magicNumber)); 
    updateSleepTime(TOTAL_SLEEP_DURATION);
  }
  
  ESP.rtcUserMemoryRead(RTC_PING_COUNT, &dsCount, sizeof(dsCount)); 
  ESP.rtcUserMemoryRead(RTC_PING_INT, &dsPingInt, sizeof(dsPingInt)); 
  ESP.rtcUserMemoryRead(RTC_PING_LAST_INT, &dsPingLastInt, sizeof(dsPingLastInt));
  
  if (dsCount == 0) {
     updateSleepTime(TOTAL_SLEEP_DURATION);
     Serial.println("********* DO USEFUL STUFF HERE ***********");
     //magicNumber = 0;
     //ESP.rtcUserMemoryWrite(RTC_BASE, &magicNumber, sizeof(magicNumber)); 
  }
  
  if (dsCount > 1) {
    sleepTimeS = dsPingInt;
    mode = WAKE_RF_DISABLED;
    dsCount--;
  }
  else if (dsCount == 1) {
    sleepTimeS = dsPingLastInt;
    mode = WAKE_RF_DEFAULT;
    dsCount--;
  }
  ESP.rtcUserMemoryWrite(RTC_PING_COUNT, &dsCount, sizeof(dsCount)); 

  Serial.printf("dsCount = %d, dsPingInt = %d, dsPingLastInt = %d\n", dsCount,dsPingInt,dsPingLastInt);
  Serial.printf("Going into deep sleep for %d seconds with mode = %d\n",sleepTimeS,mode);
  
  delay(1000);
  
  ESP.deepSleep(sleepTimeS * 1e6, mode);
}

void loop() {
}

