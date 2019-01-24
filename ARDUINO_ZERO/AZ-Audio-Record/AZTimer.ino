
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

void enableTimer() {
  TcCount16* TC = (TcCount16*) TC3;
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void disableTimer() {
  TcCount16* TC = (TcCount16*) TC3;
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void setTimerFrequency(int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

/*
This is a slightly modified version of the timer setup found at:
https://github.com/maxbader/arduino_tools
 */
void startTimer(int frequencyHz) {
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID (GCM_TCC2_TC3)) ;
  while ( GCLK->STATUS.bit.SYNCBUSY == 1 );

  TcCount16* TC = (TcCount16*) TC3;

  //TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  disableTimer();

  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  // Set prescaler to 8
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  setTimerFrequency(frequencyHz);

  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);

  //TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  enableTimer();
  
  //while (TC->STATUS.bit.SYNCBUSY == 1);
}

void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  // If this interrupt is due to the compare register matching the timer count
  // we toggle the LED.
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    
    digitalWrite(13, !digitalRead(13));
    
    //check if ADC result is ready
    while (REG_ADC_STATUS & ADC_STATUS_SYNCBUSY);
    
    adcvalue = REG_ADC_RESULT;//read ADC();
    adcvalue = adcvalue >> 2 ; //10 bit to 8 bit resolution
    //analogRead(A3); Results in max sample rate of 2 KHz
    //readADC() ; Results in max sample rate of 100 Hz

//    if ( adcvalue > 220 ) 
//     Serial.println("****************");
//     else if ( adcvalue > 200 ) 
//     Serial.println("+++++++++++++");  
//     else if ( adcvalue > 180 ) 
//     Serial.println(">>>>>>>>>"); 
//     else if ( adcvalue > 160 ) 
//     Serial.println("<<<<");  
//     else if ( adcvalue > 140 ) 
//     Serial.println("==");
//     else if ( adcvalue > 120 ) 
//     Serial.println("*");

    recByteCount++; // increment sample counter

//     if (bufsel == 0) {
//      //if (writebuf0 == 1) Serial.println("buf0 overflow");
//      buf0[idx++]=count++;
//      if (idx > 4095) {
//        writebuf0=1;
//        bufsel=1;
//        idx=0;
//      }
//    }
//    else {
//      //if (writebuf1 == 1) Serial.println("buf1 overflow");
//      buf1[idx++] = count++;
//      if (idx > 4095) {
//        writebuf1=1;
//        bufsel=0;
//        idx=0;
//      }
//    }
    
    if (recByteCount % totalBuf < bufSplit_1) {
      buf00[recByteCount % darBufSize] = adcvalue;
    } else if (recByteCount % totalBuf < bufSplit_2) { 
      buf01[recByteCount % darBufSize] = adcvalue;
    } else if (recByteCount % totalBuf < bufSplit_3) {
      buf02[recByteCount % darBufSize] = adcvalue;
    } else if (recByteCount % totalBuf < bufSplit_4) {
      buf03[recByteCount % darBufSize] = adcvalue;
    } else if (recByteCount % totalBuf < bufSplit_5) {
      buf04[recByteCount % darBufSize] = adcvalue;
    } else if (recByteCount % totalBuf < bufSplit_6) {
      buf05[recByteCount % darBufSize] = adcvalue;
    } else if (recByteCount % totalBuf < bufSplit_7) {
      buf06[recByteCount % darBufSize] = adcvalue;
    } else {
      buf07[recByteCount % darBufSize] = adcvalue;
    }
  }
}
