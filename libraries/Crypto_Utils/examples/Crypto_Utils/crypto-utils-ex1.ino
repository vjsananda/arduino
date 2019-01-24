
#include "aes256.h" //Include library files
#include <crypto-utils.h>

String str = "{msg:'i love dogs',rating:5}" ;
String tmp ;
aes256_context ctxt;

uint8_t buf[200] ;

void setup() {
  int start_idx ;
  int end_idx ;
  int len ;
  int i , j;
  String padStr , msg;
  String randKey ;
  uint8_t key[32];
  int keylen ;

  Serial.begin(115200);

  randKey = genRandomKey();
  keylen = stringToUintBuf(randKey, key);

  aes256_init(&ctxt, key);

   padStr = pad16Byte(str);
   Serial.println("padded str = " + padStr);
   Serial.println("padded str len = " + String(padStr.length()));

  len = stringToUintBuf(padStr, buf);

  for(int i=0;i<len;i += 16)
    aes256_encrypt_ecb(&ctxt, buf+i );

  String encrypted = uintBufToString(buf,len);
  Serial.println("encrypted = " + encrypted );

  len = stringToUintBuf(encrypted, buf);
  for(int i=0;i<len;i += 16)
    aes256_decrypt_ecb(&ctxt, buf+i);
  String decrypted = uintBufToString(buf,len);
  Serial.println("decrypted = " + decrypted );

  msg = extractJson(padStr);
  Serial.println("msg = " + msg);

  len = stringToUintBuf("i love dogs, I love scooby", buf);
  tmp = uintBufToString(buf, len);
  Serial.println("tmp = " + tmp );

  Serial.flush();

  delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:

}
