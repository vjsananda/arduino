/*macro definition of water sensor and the buzzer*/
#define WATER_SENSOR 5

void setup()
{
  Serial.begin(9600);
  pins_init();
}
void loop()
{
  if(isExposedToWater())
    soundAlarm();
}

void pins_init()
{
  pinMode(WATER_SENSOR, INPUT);
  //pinMode(BUZZER, OUTPUT);
}
/************************************************************************/
/*Function: When the sensor is exposed to the water, the buzzer sounds  */
/*      for 2 seconds.                        */
void soundAlarm()
{
    Serial.println("Water detected");
    delay(50);
}
/************************************************************************/
/*Function: Determine whether the sensor is exposed to the water    */
/*Parameter:-void                                 */
/*Return: -boolean,if it is exposed to the water,it will return true. */
boolean isExposedToWater()
{
  if(digitalRead(WATER_SENSOR) == LOW)
    return true;
  else return false;
}
