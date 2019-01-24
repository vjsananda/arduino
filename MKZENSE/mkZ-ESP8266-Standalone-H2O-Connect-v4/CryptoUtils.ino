byte getRandomByte() {
  return random(48,122);
}

void printByteBuf( byte * in ,unsigned int len ) {
    int i;
    for(i=0;i<len;i++)
      Serial.print( String(in[i],HEX) + " " );
    Serial.println();
}

void printUint8Buf( uint8_t * in ,unsigned int len ) {
    int i;
    for(i=0;i<len;i++)
      Serial.print( String(in[i],HEX) + " " );
    Serial.println();
}

//Given Id , creates garbled id in out[]
//Returns number of bytes in out
//Id is 32 bits, or 4 bytes
//Garble algorithm
//1. First numpreamble bytes are random
//2. Generate random byte as mask
//3. Store mask byte
//4. Store ~(mask ^ idbyte)
//5. Go on to the next id byte, to step 2
int garbleId( int numpreamble, unsigned int id, byte * out ) {

  int i;
  byte idbyte, mask;
  int wptr;

  //Fill the the first numpreamble bytes with
  //random bytes
  for(i=0;i<numpreamble;i++)
    out[i] = getRandomByte();
  wptr = i;

  //Now for the 32 bit Id
  //4 bytes
  for(i=0;i<4;i++) {
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

//Convert a string to a buffer of unsigned 8 bit 
int stringToUintBuf(String str, uint8_t * buf ) {
  int len = str.length() + 1;
  str.toCharArray(cbuf, len);
  for (int i = 0; i < len; i++) {
    buf[i] = (uint8_t)cbuf[i];
  }
  return len;
}

//Convert unsigned 8 bit to string
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

//Pad string to nearest 16 byte multiple
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

//Generate IV array of bytes from string
void generateIV(String str , uint8_t * buf) {
  str.toCharArray(buffer,17);
  for (int i=0;i<16;i++) {
    buf[i] = (uint8_t)buffer[i];  
    if ( i % 2 ) {
      buf[i] = buf[i] % 16 ;
    }
    else {
      buf[i] = 15 - (buf[i] % 16);
    }
  }
}


