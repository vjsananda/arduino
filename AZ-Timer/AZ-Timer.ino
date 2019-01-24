
#include "SdFat.h"
#include <SPI.h>
#include "FreeStack.h"

SdFat sd;
SdFile rec;

byte buf0[4096];
byte buf1[4096];
int idx=0;
byte bufsel=0;
byte count = 0;
byte writebuf0=0;
byte writebuf1=0;

#define LED_PIN 13
#define POT_PIN 2

#define POT_JITTER_THRESHOLD 10
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);
int frequencyFromPotValue(int potValue);
void TC3_Handler();

int potValue = 0;
bool isLEDOn = false;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  potValue = analogRead(POT_PIN);
  //startTimer(frequencyFromPotValue(potValue));
  startTimer(5000);
    //digitalWrite(ledStart, HIGH);
  if (!sd.begin(10, SD_SCK_MHZ(50))) {
    Serial.println("SD Card init FAILED");
    return;
  }
  else {
    Serial.println("SD Card Init SUCCESS");
    rec.open("test.dat",  O_CREAT | O_TRUNC | O_RDWR);
    sd.ls(LS_R | LS_DATE | LS_SIZE);
  }
  Serial.println("Setup complete");
}

int totalbytes = 0;

void loop() {
  //int newPotValue = analogRead(POT_PIN);
  // We only want to update the frequency if there has been significant changes
  // to the potentiometer. We do this to prevent jitters but also to make this
  // loop more efficient.
  //if (abs(newPotValue - potValue) > POT_JITTER_THRESHOLD) {
    //potValue = newPotValue;
    //setTimerFrequency(frequencyFromPotValue(potValue));
  //}
  if (writebuf0) rec.write(buf0,4096);
  if (writebuf1) rec.write(buf1,4096);

  Serial.println("totalbytes = " + String(totalbytes));
  if (totalbytes > 8192 ) {
    rec.sync();
    rec.close();
    Serial.println("END: Totalbytes=" + String(totalbytes));
    sd.ls(LS_R | LS_DATE | LS_SIZE);
    while(1) ;
  }
  
}

int frequencyFromPotValue(int potValue) {
  // potValue is between 0 and 1023 so we scale it to 1Hz -> 34Hz
  return (potValue / 30) + 1;
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

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;

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

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  // If this interrupt is due to the compare register matching the timer count
  // we toggle the LED.
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    digitalWrite(LED_PIN, isLEDOn);
    isLEDOn = !isLEDOn;
    totalbytes++;
    if (totalbytes <= 8192)
    if (bufsel == 0) {
      buf0[idx++]=count++;
      if (idx > 4095) {
        writebuf0=1;
        writebuf1=0;
        bufsel=1;
        idx=0;
      }
    }
    else {
      buf1[idx++] = count++;
      if (idx > 4095) {
        writebuf1=1;
        writebuf0=0;
        bufsel=0;
        idx=0;
      }
    }
  }
}
