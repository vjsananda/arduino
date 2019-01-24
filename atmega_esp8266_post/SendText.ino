void send_text_message(String msg) {
   
   WifiPowerUp();
   delay(2000/CLK_DIV);

    create_tcp_conn();

    String preamble = "GET /apps/thinghttp/send_request?api_key=YNUF28WWU6S33RZH&number=5126570021&message=";
    String postamble = " HTTP/1.0\r\n\r\n";
    String GET_REQUEST = preamble + msg + postamble;

    #ifdef DBG_PRINT
    Serial.print(GET_REQUEST);
    Serial.println("");
    Serial.print(F("REQUEST length = "));
    Serial.println(GET_REQUEST.length());
    Serial.flush();
    #endif
    
    wifi.send((const uint8_t*)GET_REQUEST.c_str(), GET_REQUEST.length());
    
    wifi.recvEcho(3000);
    
    
    WifiPowerDown();

    delay(TIME_BETWEEN_TEXT_MESSAGES*1000/CLK_DIV);
    
    //initializeFilters();
  
}
