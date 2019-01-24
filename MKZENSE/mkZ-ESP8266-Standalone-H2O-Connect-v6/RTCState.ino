
void rtcUpdateState( uint32_t _action ) {
   uint32_t action = _action ;
   ESP.rtcUserMemoryWrite(RTC_PIN_ACTION, &action, sizeof(action));
}

uint32_t rtcGetState(  ) {
   uint32_t action  ;
   ESP.rtcUserMemoryRead(RTC_PIN_ACTION, &action, sizeof(action));
   return action ;
}
    
void resetErrorCount() {
  uint32_t count = 0;
  ESP.rtcUserMemoryWrite(RTC_ERR_COUNT, &count, sizeof(count));  
}

uint32_t getErrorCount() {
  uint32_t count = 0;
  ESP.rtcUserMemoryRead(RTC_ERR_COUNT, &count, sizeof(count));  
  return count;
}

void incrementErrorCount() {
  uint32_t count ;
  ESP.rtcUserMemoryRead(RTC_ERR_COUNT, &count, sizeof(count));  
  count++;
  ESP.rtcUserMemoryWrite(RTC_ERR_COUNT, &count, sizeof(count));  
}

//Sleep time after error (max of 3600 sec)
//Varies based on pending action, error type and error count
uint32_t sleepTimeAfterError( uint32_t pendingAction ) {

  uint32_t errorCount = getErrorCount();
  #ifdef DBG_PRINT
  Serial.printf("** Error Count = %d **\n",errorCount);
  #endif

  if (pendingAction == PING_ACTION) {
    //Retry in 20 seconds
    if (errorCount < 3 ) return 20 ;
    else if (errorCount < 6) return 40 ;
    else return 3600 ;
  }

  if (pendingAction == ALARM_ACTION) {
    //Retry in 20 seconds
    if (errorCount < 3 ) return 20 ;
    else if (errorCount < 6) return 40 ;
    else if (errorCount < 10) return 200 ;
    else if (errorCount < 20) return 600 ;
    else if (errorCount < 40) return 1800 ;
    else return 3600 ;
  }

  return 20;
  
}

