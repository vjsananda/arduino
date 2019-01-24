void create_tcp_conn() {

     while(1) {
      if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
         #ifdef DBG_PRINT
         Serial.print(F("create tcp ok\r\n"));
         #endif
         break ;
      } else {
         #ifdef DBG_PRINT
         Serial.print(F("create tcp err\r\n"));
         #endif
      }
      delay(1000);
     }
}
