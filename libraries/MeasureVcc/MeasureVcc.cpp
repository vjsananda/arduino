
#include "MeasureVcc.h"

//const long InternalReferenceVoltage = 1082;//just this value to your board's specific internal BG voltage
//
// Code courtesy of "Coding Badly" and "Retrolefty" from the Arduino forum
// results are in millivolts
// So for example, 5V would be 5000.

int getBandgap (const long InternalReferenceVoltage)
  {
  // REFS0 : Selects AVcc external reference
  // MUX3 MUX2 MUX1 : Selects 1.1V (VBG)
   ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
   ADCSRA |= bit( ADSC );  // start conversion
   while (ADCSRA & bit (ADSC))
     { }  // wait for conversion to complete
   int results = (((InternalReferenceVoltage * 1024) / ADC) + 5) / 10;
   return results*10;
  } // end of getBandgap
