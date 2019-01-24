#define POT_JITTER_THRESHOLD 10
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

bool isLEDOn = 0;
//void setup() {
  //pinMode(LED_PIN, OUTPUT);
  //startTimer(frequencyFromPotValue(potValue));
 // startTimer(10);
//}

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

  // Set prescaler to 1024
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
    //digitalWrite(LED_PIN, isLEDOn);
    isLEDOn = !isLEDOn;
    digitalWrite(13, isLEDOn);
    
    //check if ADC result is ready
    while (REG_ADC_STATUS & ADC_STATUS_SYNCBUSY);
    
    adcvalue = REG_ADC_RESULT;//readADC();
    
    //analogRead(A3); Results in max sample rate of 2 KHz
    //readADC() ; Results in max sample rat of 100 Hz
    
    Serial.println("adcvalue = " + String(adcvalue));
  }
}
