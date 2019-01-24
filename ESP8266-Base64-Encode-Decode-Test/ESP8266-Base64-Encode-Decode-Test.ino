#include "ESPBase64.h"

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  char b64data[256]; // Size is just an example.
  //String s = "Hello world!,\r Goodbye cruel\n world";
  String s = "ABCDEF";
  Serial.println(" Message: " );
  Serial.println( s) ;

  //Encoding
  Serial.println(" Encoded message:");
  int b64len = base64_encode(b64data, (char *)s.c_str(), s.length());
  Serial.println ( String(b64data) );
  Serial.println ("The length is: " + String(b64len) );
  
  //Decoding:
  char decoded[256];
  String ss("Yjl7ImUiOjAsImkiOjk5LCJtIjozMiwicCI6MTYsImMiOjB9YkhET1RKalhieT1rWU09eEdNMndGYGRaTlZWTg==");
  //String ss(b64data);
  base64_decode( decoded , (char *)ss.c_str() , ss.length() );
  Serial.println("Decoded: " + String(decoded));


  //base64_decode( decodec, b64data, b64len);
  //Serial.println("Decoded: " + String(decoded));

}

void loop() {
  // put your main code here, to run repeatedly:

}
