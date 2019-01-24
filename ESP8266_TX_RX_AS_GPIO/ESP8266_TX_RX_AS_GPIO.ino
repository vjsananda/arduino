/*
 * Sketch to test if we can use TX and RX pins as GPIO
 * TX : GPIO1
 * RX : GPIO3
 */

 #define TX_PIN 1
 #define RX_PIN 3
 
void setup() {
  // put your setup code here, to run once:
  pinMode(TX_PIN,FUNCTION_3);
  pinMode(RX_PIN,FUNCTION_3);
  
  pinMode(TX_PIN,OUTPUT);
  pinMode(RX_PIN,OUTPUT);

  //Delay, disconnect FTDI before RX turns around
  delay(10000);
}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(TX_PIN,0);
  digitalWrite(RX_PIN,0);
  delay(1000);
  
  digitalWrite(TX_PIN,0);
  digitalWrite(RX_PIN,1);
  delay(1000);
  
  digitalWrite(TX_PIN,1);
  digitalWrite(RX_PIN,0);
  delay(1000);
  
  digitalWrite(TX_PIN,1);
  digitalWrite(RX_PIN,1);
  delay(1000);
  
}
