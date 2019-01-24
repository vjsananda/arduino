#ifndef _CRYPTO_UTILS_H_
#define _CRYPTO_UTILS_H_

#include <Arduino.h>
#include <String.h>

#define MAX_STRING_LENGTH 200

int stringToUintBuf(String str, uint8_t * buf ) ;

String uintBufToString (uint8_t * buf , int len);

String extractJson(String str) ;

String pad16Byte(String str);

String genRandomKey() ;

#endif
