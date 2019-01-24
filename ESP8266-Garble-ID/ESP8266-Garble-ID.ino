
byte buf[64];

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

//Ungarbles Id, given in buf
//Returns ungarbled Id
unsigned int unGarbleId( int numpreamble, byte * in , int len) {
  int rptr = numpreamble ;
  byte mask ;
  byte idbyte ;
  unsigned int rval = 0;
  int i;
  for(i=0;i<4;i++) {
    mask = in[rptr++];
    //Serial.println("Mask = " + String(mask,HEX));
    idbyte = in[rptr++];
    //Serial.println("idbyte = " + String(idbyte,HEX));
    rval |= ((~idbyte ^ mask) & 0x0ff) << (8*i) ;    
  }
  return rval ;
}

void setup() {
  int i ;
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(" ");

  unsigned int id = 0xdeadbeef ;

  for (i =0;i<32;i++) {
  
  Serial.println("Input Id = " + String(id,HEX));

  int len = garbleId(8, id , buf ) ;

  Serial.println("Garbled Id = ");
  printByteBuf(buf,len);

  unsigned int ungarbledId = unGarbleId(8, buf, len) ;

  Serial.println("Ungarbled id = " + String(ungarbledId,HEX) + "\n\n");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
