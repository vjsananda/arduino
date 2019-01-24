void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(12,INPUT);
  pinMode(11,INPUT);
}

int value12;
int value11;

void loop() {
  // put your main code here, to run repeatedly:
  value11 = digitalRead(11);
  value12 = digitalRead(12);

  Serial.print("Value = ");
  Serial.print(value11);
  Serial.print(value12);
  Serial.println(" " );
  delay(1000);
  
}
