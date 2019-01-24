//This sketch is from a tutorial on the ForceTronics YouTube Channel called 
//Utilizing Advanced ADC Capabilities on Arduino’s with the SAMD21 (Zero, MKR1000, etc)
//This code is public domain and free to anyone to use or modify at your own risk

//Youtube video on advanced use of ADC on Arduino Zero. Useful in figuring how to program ADC at low level. 
// https://www.youtube.com/watch?v=QOYSGCcY6mQ&list=WL&index=61


//This is the interrupt service routine (ISR) that is called 
//if an ADC measurement falls out of the range of the window 
void ADC_Handler() {
    //digitalWrite(LED_BUILTIN, HIGH); //turn LED on
    adcvalue = readADC();
    Serial.println("**adc exceeded threshold** :" + String(adcvalue));
    ADC->INTFLAG.reg = ADC_INTFLAG_WINMON; //Need to reset interrupt
}

//this function sets up the ADC window mode with interrupt
void ADCWindowBegin(byte mode, int upper, int lower) {
  setMeasPin(); //function sets up ADC pin A0 as input
  setGenClock(); //setup ADC clock, using internal 8MHz clock
  setUPADC(); //configure ADC
  setADCWindow(mode, upper, lower); //setup ADC window mode 
  setUpInterrupt(0); //setup window mode interrupt with highest priority
  enableADC(1); //enable ADC 
}

void ADCWindowEnd() {
  NVIC_DisableIRQ(ADC_IRQn); //turn off interrupt
  enableADC(0); //disable ADC 
}

//setup measurement pin, using Arduino ADC pin A3
void setMeasPin() {
  // Input pin for ADC Arduino A3/PA04
  REG_PORT_DIRCLR1 = PORT_PA04;

  // Enable multiplexing on PA04
  PORT->Group[0].PINCFG[4].bit.PMUXEN = 1;
  PORT->Group[0].PMUX[1].reg = PORT_PMUX_PMUXE_B | PORT_PMUX_PMUXO_B;
}

//Function sets up generic clock for ADC
//Uses built-in 8MHz clock
void setGenClock() {
   // Enable the APBC clock for the ADC
  REG_PM_APBCMASK |= PM_APBCMASK_ADC;

  configOSC8M(); //this function sets up the internal 8MHz clock that we will use for the ADC
  
  // Setup clock GCLK3 for no div factor
   GCLK->GENDIV.reg |= GCLK_GENDIV_ID(3)| GCLK_GENDIV_DIV(1);
   while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);  

  //configure the generator of the generic clock, which is 8MHz clock
  GCLK->GENCTRL.reg |= GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_ID(3) | GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
  
  //enable clock, set gen clock number, and ID to where the clock goes (30 is ADC)
  GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(3) | GCLK_CLKCTRL_ID(30);
  while (GCLK->STATUS.bit.SYNCBUSY);
}

//Function that does general settings for ADC
//sets it for a single sample
//Uses internal voltage reference
//sets gain factor to 1/2
void setUPADC() {
  // Select reference, internal VCC/2
  ADC->REFCTRL.reg |= ADC_REFCTRL_REFSEL_INTVCC1; // VDDANA/2, combine with gain DIV2 for full VCC range

  // Average control 1 sample, no right-shift
  ADC->AVGCTRL.reg |= ADC_AVGCTRL_ADJRES(0) | ADC_AVGCTRL_SAMPLENUM_1;

  // Sampling time, no extra sampling half clock-cycles
  REG_ADC_SAMPCTRL |= ADC_SAMPCTRL_SAMPLEN(0);

  // Input control: set gain to div by two so ADC has measurement range of VCC, no diff measurement so set neg to gnd, pos input set to pin 0 or A0
  ADC->INPUTCTRL.reg |= ADC_INPUTCTRL_GAIN_DIV2 | ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_PIN4;
  while (REG_ADC_STATUS & ADC_STATUS_SYNCBUSY);

  // PS16, 8 MHz, ADC_CLK = 500 kHz, ADC sampling rate, single eded, 12 bit, free running, DIV2 gain, 7 ADC_CLKs, 14 usec
  ADC->CTRLB.reg |= ADC_CTRLB_PRESCALER_DIV16 | ADC_CTRLB_RESSEL_10BIT | ADC_CTRLB_FREERUN; // Run ADC continously, 7 ADC_CLKs, 14 usec
  while (REG_ADC_STATUS & ADC_STATUS_SYNCBUSY);
}

//This function is used to setup the ADC windowing mode
//inputs are the mode, upper window value, and lower window value
//
void setADCWindow(byte mode, int upper, int lower) {
  ADC->WINCTRL.reg = mode; //set window mode
  while (ADC->STATUS.bit.SYNCBUSY);

   ADC->WINUT.reg = upper; //set upper threshold
   while (ADC->STATUS.bit.SYNCBUSY);

   ADC->WINLT.reg = lower; //set lower threshold
   while (ADC->STATUS.bit.SYNCBUSY);
}

//This function sets up an ADC interrupt that is triggered 
//when an ADC value is out of range of the window
//input argument is priority of interrupt (0 is highest priority)
void setUpInterrupt(byte priority) {
  
  ADC->INTENSET.reg |= ADC_INTENSET_WINMON; // enable ADC window monitor interrupt
   while (ADC->STATUS.bit.SYNCBUSY);

   NVIC_EnableIRQ(ADC_IRQn); // enable ADC interrupts
   NVIC_SetPriority(ADC_IRQn, priority); //set priority of the interrupt
}

//function allows you to enable or disable ADC
void enableADC(bool en) {
  if(en) ADC->CTRLA.reg = 2; //2 is binary 010 which is register bit to enable ADC
  else ADC->CTRLA.reg = 0; //0 disables ADC
}

//This function will return the latest ADC reading made during free run window mode
//must first start the ADC before calling this function
unsigned int readADC() {
  // Free running, wait for conversion to complete
  while (!(REG_ADC_INTFLAG & ADC_INTFLAG_RESRDY));
  // Wait for synchronization before reading RESULT
  while (REG_ADC_STATUS & ADC_STATUS_SYNCBUSY);
  
  return REG_ADC_RESULT;
}

//function enables the 8MHz clock used for the ADC
void configOSC8M() 
{
  SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ENABLE;
}

