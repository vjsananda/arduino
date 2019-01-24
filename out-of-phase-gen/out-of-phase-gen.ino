byte phase0 = 12;
byte phase1 = 11;
byte value = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(phase0,OUTPUT);
  pinMode(phase1,OUTPUT);
  digitalWrite(phase0,LOW);
  digitalWrite(phase1,LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(phase0,HIGH);
  digitalWrite(phase0,LOW);
 digitalWrite(phase1,HIGH);
  digitalWrite(phase1,LOW); 
}
