#include "MeasureVcc.h"

#define INTERNAL_REF_VOLTAGE  1082 //just this value to your board's specific internal BG voltage

void setup(void)
{
  Serial.begin(9600);
}
    
void loop(void)
{
  Serial.println (getBandgap(INTERNAL_REF_VOLTAGE));
  delay(1000);
}

