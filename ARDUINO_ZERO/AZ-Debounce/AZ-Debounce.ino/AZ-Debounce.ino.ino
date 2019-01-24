void setup() {
  pinMode(13,OUTPUT);
  pinMode(2,INPUT);
  // put your setup code here, to run once:

}

bool LEDOn = 0;

bool buttonPressed() {
  int count = 0;
  for(int i=0;i<100;i++) {
    byte value = digitalRead(2);
    if (value == LOW) count++;
    else count--;
    delay(1);
  } 
  return count > 0;
}


bool buttonReleased() {
  int count = 0;
  for(int i=0;i<25;i++) {
    byte value = digitalRead(2);
    if (value == HIGH) count++;
    else count--;
    delay(1);
  } 
  return count > 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  if ( buttonPressed())
    LEDOn = !LEDOn;

  if (LEDOn)
    digitalWrite(13,HIGH);
  else
    digitalWrite(13,LOW);

  delay(500);
}
