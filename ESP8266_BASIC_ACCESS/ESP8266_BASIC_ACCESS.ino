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

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600); // your esp's baud rate might be different
  pinMode(ch_pd,OUTPUT);
  digitalWrite(ch_pd,HIGH);
}

void loop()
{
  if(Serial1.available()) // check if the esp is sending a message
  {
    while(Serial1.available())
    {
      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      Serial.write(c);
    }
  }


  if(Serial.available())
  {
    // the following delay is required because otherwise the arduino will read the first letter of the command but not the rest
    // In other words without the delay if you use AT+RST, for example, the Arduino will read the letter A send it, then read the rest and send it
    // but we want to send everything at the same time.
    //delay(1000);

    String command="";

    while(Serial.available()) // read the command character by character
    {
        // read one character
      command+=(char)Serial.read();
    }
    Serial1.println(command); // send the read character to the Serial1
  }
}
