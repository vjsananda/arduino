// ----------------------------------------------------
// Adapted from :
// APC magazine - Arduino Masterclass
// Project #18 - Digital Audio Recorder v2.0
// Darren Yates - 11 April 2014
//
// Includes the SdFat library - https://code.google.com/p/sdfatlib/downloads/list
//
// ----------------------------------------------------

#include "SdFat.h"
#include <SPI.h>
#include "FreeStack.h"

SdFat sd;
SdFile rec;

const unsigned int darBufSize = 512;
const unsigned int totalBuf = darBufSize * 8;
const unsigned int bufSplit_1 = darBufSize;
const unsigned int bufSplit_2 = darBufSize * 2;
const unsigned int bufSplit_3 = darBufSize * 3;
const unsigned int bufSplit_4 = darBufSize * 4;
const unsigned int bufSplit_5 = darBufSize * 5;
const unsigned int bufSplit_6 = darBufSize * 6;
const unsigned int bufSplit_7 = darBufSize * 7;

//------------ SET SAMPLE RATE HERE --------------------
unsigned long sampleRate = 22000;
//------------ SET SAMPLE RATE HERE --------------------

const int SDchipSelect = 10;

unsigned long bytesPerSec = sampleRate;
unsigned int blockAlign = 1;
unsigned int bitsPerSample = 8;
unsigned long dataSize = 0L;
unsigned long recByteCount = 0L;
unsigned long recByteSaved = 0L;
unsigned long fileSize = 0L;
unsigned long waveChunk = 16;
unsigned int waveType = 1;
unsigned int numChannels = 1;
const int btn = 2;
const int ledStart = 13;
unsigned long oldTime = 0L;
unsigned long newTime = 0L;

byte buf00[darBufSize]; // buffer array 1
byte buf01[darBufSize]; // buffer array 2
byte buf02[darBufSize]; // buffer array 3
byte buf03[darBufSize]; // buffer array 4
byte buf04[darBufSize]; // buffer array 4
byte buf05[darBufSize]; // buffer array 4
byte buf06[darBufSize]; // buffer array 4
byte buf07[darBufSize]; // buffer array 4
byte byte1, byte2, byte3, byte4;

byte buf0[4096];
byte buf1[4096];
int idx=0;
byte bufsel=0;
byte count = 0;
byte writebuf0=0;
byte writebuf1=0;

unsigned int bufByteCount;
byte bufWrite;
byte lastBuf = 7;

int RECORDING  = 0;

//declare const for window mode settings
const byte DISABLE = 0;
const byte MODE1 = 1; //RESULT > Lower Tolerance
const byte MODE2 = 2; //RESULT < Upper Tolerance
const byte MODE3 = 3;
const byte MODE4 = 4;
int adcvalue = 0;

void setup() { // THIS RUNS ONCE
  Serial.begin(9600);
  //  pinMode(10, OUTPUT);
  pinMode(ledStart, OUTPUT);
  pinMode(btn, INPUT_PULLUP);
  //  pinMode(ledStop, OUTPUT);
  //  pinMode(btnStop, INPUT_PULLUP);
  //  pinMode(btnStart, INPUT_PULLUP);

  //call this function to start the ADC in window and define the window parameters
  //ADCWindowBegin(mode, upper, lower)
  //In call below using MODE1, the upper limit (1023) is a don't care,
  //an interrupt is triggered each time the ADC value exceed the Lower limit (200)
  //For audio recording use it to figure out when the microphone audio output is saturating the
  //Analog to digital convertor
  //Using windowed mode was causing trouble with high sampling rates. Disabling for now.
  ADCWindowBegin(DISABLE, 1023, 1022); //Do not use the Arduino analog functions until you call ADCWindowEnd()
  startTimer(sampleRate);
  disableTimer();

  //digitalWrite(ledStart, HIGH);
  if (!sd.begin(SDchipSelect, SD_SCK_MHZ(50))) {
    Serial.println("SD Card init FAILED");
    return;
  }
  else {
    Serial.println("SD Card Init SUCCESS");
    sd.ls(LS_R | LS_DATE | LS_SIZE);
  }
}

void loop() { 

  if (buttonPressed(btn) && RECORDING == 0) {
    Serial.println("Recording >>>>>>>> START <<<<<<<<<");
    digitalWrite(ledStart, HIGH);
    RECORDING = 1;
    StartRec(); // launch StartRec method
  }
  else if (buttonPressed(btn) && RECORDING == 1) {
    digitalWrite(ledStart, LOW);
    RECORDING = 0;
    StopRec(); // launch StopRec method
    Serial.println("Recording ******* STOPPED *******");
  }

  //Serial.println("recByteCount = " + String(recByteCount));

  if (RECORDING == 1) {
  
    if (recByteCount % totalBuf > bufSplit_1 && lastBuf == 7) {
      rec.write(buf00, darBufSize); 
      Serial.println("writing ringbuf 0");
      rec.sync();
      lastBuf = 0; recByteSaved += darBufSize;
    } else if (recByteCount % totalBuf > bufSplit_2 && lastBuf == 0) {
      rec.write(buf01, darBufSize);
      Serial.println("writing ringbuf 1");
      rec.sync();
      lastBuf = 1; recByteSaved += darBufSize;
    } else if (recByteCount % totalBuf > bufSplit_3 && lastBuf == 1) {
      rec.write(buf02, darBufSize); 
      Serial.println("writing ringbuf 2");
      rec.sync();
      lastBuf = 2; recByteSaved += darBufSize;
    } else if (recByteCount % totalBuf > bufSplit_4 && lastBuf == 2) {
      rec.write(buf03, darBufSize);
      Serial.println("writing ringbuf 3");
      rec.sync();
      lastBuf = 3; recByteSaved += darBufSize;
    } else if (recByteCount % totalBuf > bufSplit_5 && lastBuf == 3) {
      rec.write(buf04, darBufSize);
      Serial.println("writing ringbuf 4");
      rec.sync();
      lastBuf = 4; recByteSaved += darBufSize;
    } else if (recByteCount % totalBuf > bufSplit_6 && lastBuf == 4) {
      rec.write(buf05, darBufSize);
      Serial.println("writing ringbuf 5");
      rec.sync();
      lastBuf = 5; recByteSaved += darBufSize;
    } else if (recByteCount % totalBuf > bufSplit_7 && lastBuf == 5) {
      rec.write(buf06, darBufSize);
      Serial.println("writing ringbuf 6");
      rec.sync();
      lastBuf = 6; recByteSaved += darBufSize;
    } else if (recByteCount % totalBuf < bufSplit_1 && lastBuf == 6) {
      rec.write(buf07, darBufSize); 
      Serial.println("writing ringbuf 7");
      rec.sync();
      lastBuf = 7; recByteSaved += darBufSize;
    }
  }
  else
    delay(250);
}

void StartRec() { // begin recording process

  recByteCount = 0;
  bufByteCount = 0;
  recByteSaved = 0;
  bufWrite = 0;
  
  writeWavHeader();
  
  enableTimer();
  //sbi (TIMSK2, OCIE2A); // enable timer interrupt, start grabbing audio
}

void StopRec() { // stop recording process, update WAV header, close file
  digitalWrite(ledStart, LOW); // turn off recording LED
  //cbi (TIMSK2, OCIE2A); // disable timer interrupt
  disableTimer();
  writeOutUpdatedHeader(); 
  //digitalWrite(ledStop,HIGH); // light stop LED
  sd.ls(LS_R | LS_DATE | LS_SIZE);
  Serial.println("recByteCount = " + String(recByteCount));
}

// update WAV header with final filesize/datasize
void writeOutUpdatedHeader() { 
  rec.seekSet(4);
  byte1 = recByteSaved & 0xff;
  byte2 = (recByteSaved >> 8) & 0xff;
  byte3 = (recByteSaved >> 16) & 0xff;
  byte4 = (recByteSaved >> 24) & 0xff;
  rec.write(byte1);  rec.write(byte2);  rec.write(byte3);  rec.write(byte4);
  rec.seekSet(40);
  rec.write(byte1);  rec.write(byte2);  rec.write(byte3);  rec.write(byte4);
  rec.close();
}

// write out original WAV header to file
void writeWavHeader() { 

  recByteSaved = 0;
  if ( !rec.open("rec00000.wav", O_CREAT | O_TRUNC | O_RDWR)) {
    Serial.println("SD card file open failed");
  }
  rec.write("RIFF");
  byte1 = fileSize & 0xff;
  byte2 = (fileSize >> 8) & 0xff;
  byte3 = (fileSize >> 16) & 0xff;
  byte4 = (fileSize >> 24) & 0xff;
  rec.write(byte1);  rec.write(byte2);  rec.write(byte3);  rec.write(byte4);
  rec.write("WAVE");
  rec.write("fmt ");
  byte1 = waveChunk & 0xff;
  byte2 = (waveChunk >> 8) & 0xff;
  byte3 = (waveChunk >> 16) & 0xff;
  byte4 = (waveChunk >> 24) & 0xff;
  rec.write(byte1);  rec.write(byte2);  rec.write(byte3);  rec.write(byte4);
  byte1 = waveType & 0xff;
  byte2 = (waveType >> 8) & 0xff;
  rec.write(byte1);  rec.write(byte2);
  byte1 = numChannels & 0xff;
  byte2 = (numChannels >> 8) & 0xff;
  rec.write(byte1);  rec.write(byte2);
  byte1 = sampleRate & 0xff;
  byte2 = (sampleRate >> 8) & 0xff;
  byte3 = (sampleRate >> 16) & 0xff;
  byte4 = (sampleRate >> 24) & 0xff;
  rec.write(byte1);  rec.write(byte2);  rec.write(byte3);  rec.write(byte4);
  byte1 = bytesPerSec & 0xff;
  byte2 = (bytesPerSec >> 8) & 0xff;
  byte3 = (bytesPerSec >> 16) & 0xff;
  byte4 = (bytesPerSec >> 24) & 0xff;
  rec.write(byte1);  rec.write(byte2);  rec.write(byte3);  rec.write(byte4);
  byte1 = blockAlign & 0xff;
  byte2 = (blockAlign >> 8) & 0xff;
  rec.write(byte1);  rec.write(byte2);
  byte1 = bitsPerSample & 0xff;
  byte2 = (bitsPerSample >> 8) & 0xff;
  rec.write(byte1);  rec.write(byte2);
  rec.write("data");
  byte1 = dataSize & 0xff;
  byte2 = (dataSize >> 8) & 0xff;
  byte3 = (dataSize >> 16) & 0xff;
  byte4 = (dataSize >> 24) & 0xff;
  rec.write(byte1);  rec.write(byte2);  rec.write(byte3);  rec.write(byte4);
}

// Arduino Uno ATMEGA functions, had to change for SAMD21 ucontroller in ARduino Zero.
//void Setup_timer2() {
//
//  TCCR2B = _BV(CS21);  // Timer2 Clock Prescaler to : 8
//  TCCR2A = _BV(WGM21); // Interupt frequency  = 16MHz / (8 x 90 + 1) = 22191Hz
//  OCR2A = 90; // Compare Match register set to 90
//
//}

//void Setup_ADC() {
//
//  ADMUX = 0x65; // set ADC to read pin A5, ADLAR to 1 (left adjust)
//  cbi(ADCSRA,ADPS2); // set prescaler to 8 / ADC clock = 2MHz
//  sbi(ADCSRA,ADPS1);
//  sbi(ADCSRA,ADPS0);
//}

//ISR(TIMER2_COMPA_vect) {

//Not using this, the readADC() function results in a sampling rate of 100Hz
//void ADCToBuf() {
//  adcvalue = readADC();
//  recByteCount++; // increment sample counter
//  bufByteCount++;
//  if (bufByteCount == 512 && bufWrite == 0) {
//    bufByteCount = 0;
//    bufWrite = 1;
//  } else if (bufByteCount == 512 & bufWrite == 1) {
//    bufByteCount = 0;
//    bufWrite = 0;
//  }
//}

