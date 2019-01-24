void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
 Serial.setDebugOutput(true);
 pinMode(12,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello World2");
  delay(100);
  digitalWrite(12,0);
  delay(100);
  digitalWrite(12,1);
}
