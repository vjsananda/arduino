byte getRandomByte() {
  return random(48,122);
}

void printByteBuf( byte * in ,unsigned int len ) {
    int i;
    for(i=0;i<len;i++)
      Serial.print( String(in[i],HEX) + " " );
    Serial.println();
}

//Given Id , creates garbled id in out[]
//Returns number of bytes in out
int garbleId( int numpreamble, unsigned int id, byte * out ) {

  int i;
  byte idbyte, mask;
  int wptr;
  
  for(i=0;i<numpreamble;i++)
    out[i] = getRandomByte();
  wptr = i;
  
  for(i=0;i<8;i++) {
    idbyte = (id & 0xff) ;
    //Serial.println("idbyte = " + String(idbyte,HEX));
    mask = getRandomByte();  
    //Serial.println("mask = " + String(mask,HEX));
    out[wptr++] = mask;
    out[wptr++] = ~(idbyte ^ mask) ;
    id >>= 8;//shift next byte over
  }

  return wptr;//This is the total length
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
    //tmp = tmp + String((char)random(48,122));
    tmp = tmp + String((char)0 );
  return tmp;
}
