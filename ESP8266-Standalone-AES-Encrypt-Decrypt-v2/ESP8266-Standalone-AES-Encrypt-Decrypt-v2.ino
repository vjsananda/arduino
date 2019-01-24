
#include "Crypto.h"
#include "ESPBase64.h"

String str = "{msg:'i love dogs',rating:5}" ;
String tmp ;

#define MAX_STRING_LENGTH 400

uint8_t buf[MAX_STRING_LENGTH] ;
uint8_t outbuf[MAX_STRING_LENGTH] ;
char cbuf[MAX_STRING_LENGTH];

String pad16Byte(String str) {
  int i;
  int num_pad , num_pad_begin, num_pad_end ;
  String pad_begin , pad_end ;
  String padded_str ;
  int rem, quo ;
  quo = str.length() / 16 ;
  num_pad = (quo + 2) * 16 - str.length() ;

  num_pad_begin = random(num_pad / 2);
  num_pad_end = num_pad - num_pad_begin ;

  for (i = 0; i < num_pad_begin; i++)
    pad_begin = pad_begin + String( (char)random(48, 122) );

  for (i = 0; i < num_pad_end; i++)
    pad_end = pad_end + String( (char)random(48, 122) );

  padded_str = pad_begin + str + pad_end ;

  return padded_str ;
}

int stringToUintBuf(String str, uint8_t * buf ) {
  int len = str.length() + 1;
  str.toCharArray(cbuf, len);
  for (int i = 0; i < len; i++) {
    buf[i] = (uint8_t)cbuf[i];
  }
  return len;
}

String uintBufToString (uint8_t * buf , int len) {
  for (int i = 0; i < len; i++) {
    cbuf[i] = (char)buf[i];
  }
  return String(cbuf);
}

char b64data[MAX_STRING_LENGTH];

void setup() {
  Serial.begin(115200);
  uint8_t key[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
  uint8_t iv[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

  AES AES_Encrypt = AES( key, iv, AES::AES_MODE_256, AES::CIPHER_ENCRYPT);
  AES AES_Decrypt = AES( key, iv, AES::AES_MODE_256, AES::CIPHER_DECRYPT);

  String padStr = pad16Byte(str);
  Serial.println("padded str = " + padStr);
  Serial.println("padded str len = " + String(padStr.length()));

  int len = stringToUintBuf(padStr, buf);

  AES_Encrypt.process(buf, outbuf, len);  
  tmp = uintBufToString( outbuf , len );
  Serial.println("Encrypted string = "  + tmp );
  
  int b64len = base64_encode(b64data, (char *)outbuf, len) ;
  Serial.println("Encrypted string (base64) = "  + String(b64data) );

    //Decoding:
  char decoded[256];
  base64_decode( decoded , (char *)String(b64data).c_str() , String(b64data).length() );
  
  AES_Decrypt.process((uint8_t *)decoded, buf, len);
  
  tmp = uintBufToString( buf , len );
  Serial.println("Decrypted string = "  + tmp );
}

void loop() {
  // put your main code here, to run repeatedly:

}
