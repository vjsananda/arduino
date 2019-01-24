
#include "aes256.h" //Include library files

String str = "{msg:'i love dogs',rating:5}" ;
String tmp ;
aes256_context ctxt;

#define MAX_STRING_LENGTH 400

uint8_t buf[MAX_STRING_LENGTH] ;
char cbuf[MAX_STRING_LENGTH];

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

String extractJson(String str) {
  int start_idx, end_idx;
  start_idx = str.indexOf('{');
  end_idx = str.lastIndexOf('}');
  return str.substring(start_idx, end_idx + 1);
}

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

String genRandomKey() {
  int i;
  String tmp ;
  for(i=0;i<31;i++)
    tmp = tmp + String((char)random(48,122));
  return tmp;
}

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
