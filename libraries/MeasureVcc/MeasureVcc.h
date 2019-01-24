#ifndef _MEASURE_VCC_H_
#define _MEASURE_VCC_H_

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

//const long InternalReferenceVoltage = 1082;//just this value to your board's specific internal BG voltage
//
// Code courtesy of "Coding Badly" and "Retrolefty" from the Arduino forum
// results are in millivolts
// So for example, 5V would be 5000.

int getBandgap (const long InternalReferenceVoltage) ;

#ifdef __cplusplus
}
#endif

#endif
