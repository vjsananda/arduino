//Serial is the UART connected to the USB-serial port of the host PC.
//it is not connected to pins 0 and 1 of the Galileo board
//
//Serial1 is the UART connected to pins 0 and 1 of the board.
//These pins are connected to the RX & TX pins of the ESP8266 board
//
//Baud rates higher than 9600 don't work on this version of the
//ESP8266 firmware.
//The Serial monitor baud rate can be set as high as possible.
//

int ch_pd=12;
int rst=11;
int gpio_0=10;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600); // your esp's baud rate might be different
  pinMode(ch_pd,OUTPUT);
  pinMode(rst,OUTPUT);
  pinMode(gpio_0,OUTPUT);
  digitalWrite(ch_pd,HIGH);

  digitalWrite(rst,LOW);
  digitalWrite(gpio_0,LOW);
  delay(2000);
  digitalWrite(rst,HIGH);
  digitalWrite(gpio_0,LOW);
  
}

void loop()
{

    while(Serial.available()) // read the command character by character
    {
        Serial1.write(Serial.read());
    }
    
}
