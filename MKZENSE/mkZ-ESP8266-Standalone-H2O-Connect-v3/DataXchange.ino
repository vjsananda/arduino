//Return a Json string given input fields
String getJsonString( int id, int msg_id, float vcc, int alarm ) {
    
  StaticJsonBuffer<200> jsonBuffer;
  String JsonString ;

  JsonObject& root = jsonBuffer.createObject();
  root["i"] = id;
  root["v"] = vcc;
  root["a"] = alarm ;
  root["m"] = msg_id;

  root.printTo(JsonString);
  return JsonString;
}

//Encrypt and send post request
int sendPostRequest(String body) {

  int start_idx ;
  int end_idx ;
  int len ;
  int i , j;
  String padStr , msg;
  String randKey ;
  String tmp;
  int keylen ;

  //Convert char array iv_password to string
  String ivpass_str(iv_password);
  
  generateIV(ivpass_str,key);
  generateIV(ivpass_str,iv);

  Serial.println("iv");
  printUint8Buf(iv,16);
  Serial.println("\n");

  AES AES_Encrypt = AES( key, iv, AES::AES_MODE_128, AES::CIPHER_ENCRYPT);
  AES AES_Decrypt = AES( key, iv, AES::AES_MODE_128, AES::CIPHER_DECRYPT);
  
  DynamicJsonBuffer jsonBuffer;

  #ifdef DBG_PRINT
  Serial.println( "len of body = " + String(body.length()) ) ;
  #endif
  
  padStr = pad16Byte(body);

  #ifdef DBG_PRINT
  Serial.println( "padStr = " + padStr ) ;
  Serial.println( "len of padStr = " + String(padStr.length()) ) ;
  #endif
  
  len = stringToUintBuf(padStr, buf);

  AES_Encrypt.process(buf, outbuf, len);  
  tmp = uintBufToString( outbuf , len );

  #ifdef DBG_PRINT
  Serial.println("Encrypted string = "  + tmp );
  #endif
  
  int b64len = base64_encode(b64data, (char *)outbuf, len) ;
  String encrypted = String(b64data);
  
  #ifdef DBG_PRINT
  Serial.println("Encrypted string (base64) = "  + encrypted );  
  Serial.println( "len of encrypted = " + String(encrypted.length()) ) ;
  #endif

  //Garble and base64 encode the ID
  len = garbleId(8,ID, byteBuf ) ;
  Serial.println("Garbled Id bytebuf = ");
  printByteBuf(byteBuf,len);
  b64len = base64_encode(b64data, (char *)byteBuf, len) ;
  String garbledIdStr = String(b64data);

  String postBody = "{" + garbledIdStr + "}" + "[" + encrypted + "]";
  
  String preamble = "POST /postform HTTP/1.0\r\nhost:" + String(HOST_NAME) + "\r\n";
  //String header = "content-length:" + String(MAX_BODY_LENGTH) + "\r\n" ;
  String header = "content-length:" + String(postBody.length()) + "\r\n" ;
  String blank_line = "\r\n";

  //for(i=0;i<body.length();i++)
  //encrypted_body[i] = body.c_str()[i];

  //aes128_cbc_enc(key, key, encrypted_body, MAX_BODY_LENGTH);

  //String POST_REQUEST = preamble + header + blank_line + String((char *)encrypted_body) + blank_line + blank_line ;
  String POST_REQUEST = preamble + header + blank_line + postBody + blank_line + blank_line ;

#ifdef DBG_PRINT
  Serial.print(POST_REQUEST);
  Serial.println("");
  Serial.print("REQUEST length = ");
  Serial.println(POST_REQUEST.length());
#endif

  client.print(POST_REQUEST);

  delay(100);
  
  String respChar;
  String response = "";
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    respChar = client.readStringUntil('\r');
    response = response + respChar ;
  }

  delay(100);

  #ifdef DBG_PRINT
  Serial.println("SERVER RESPONSE: \n" + response);
  #endif

  int body_start_idx = response.indexOf("[");
  int body_end_idx = response.lastIndexOf("]");

  response = response.substring( body_start_idx+1,body_end_idx );

  #ifdef DBG_PRINT
  Serial.println("BODY RESPONSE: \n" + response);
  #endif
  
  char decoded[256];
  int decodelen = base64_decode(decoded, (char *)response.c_str(), response.length() ) ;

  #ifdef DBG_PRINT
  Serial.println("Base64 decoded bytes : ");
  for(int i=0;i<decodelen;i++)
    Serial.print( String( decoded[i], HEX) + " "  ) ;
  Serial.println("\n");
  #endif
  
  //String decodeString = String(decoded);
  #ifdef DBG_PRINT
  //Serial.println("Base64 decode : " + decodeString );
  #endif
  
  //AES_Decrypt.process((uint8_t *)decodeString.c_str(), buf, decodeString.length() );
  AES_Decrypt.process((uint8_t *)decoded, buf, decodelen);
  //String decryptedString  = uintBufToString( buf , decodelen.length() );
  String decryptedString  = uintBufToString( buf , decodelen );
  #ifdef DBG_PRINT
  Serial.println("Decrypted string = "  + decryptedString );
  #endif
  
  String jsonResponse = decryptedString.substring( decryptedString.indexOf("{"), decryptedString.lastIndexOf("}")+1 );
  
  Serial.println("JSON RESPONSE : " + jsonResponse);
  
  JsonObject & root = jsonBuffer.parseObject(jsonResponse);

   if (!root.success())
   {
     Serial.println("parseObject() failed");
     return 0;
   }

   int e,m,cmd;
   int p;

   //i = root["i"];
   e = root["e"];
   m = root["m"];
   cmd = root["c"];
   p = root["p"];

   //Serial.print("i=");
   //Serial.println(i);

   #ifdef DBG_PRINT
   Serial.print("e=");
   Serial.println(e);

   Serial.print("m=");
   Serial.println(m);

   Serial.print("c=");
   Serial.println(cmd);

   Serial.print("p=");
   Serial.println(p);

   Serial.flush();
   #endif

   //If no error then ping after ping interval
   if (e == 0) {
        updateSleepTime(p);
   }
   else {
    updateSleepTime(SLEEP_TIME_IF_ERROR);
   }
   return e;
}
