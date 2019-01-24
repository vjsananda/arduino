void esp_cold_reset() {
  digitalWrite(ESP_RST_PIN,LOW);
  delay(2000);
  digitalWrite(ESP_RST_PIN,HIGH); 
}

void WifiPowerUp() {
  #ifdef DBG_PRINT
  Serial.println(F("** Power Up Wifi **"));
  #endif
  
  digitalWrite(ch_pd,HIGH);
}

void WifiPowerDown() {
  #ifdef DBG_PRINT
  Serial.println(F("-- Power Down Wifi --"));
  #endif
  digitalWrite(ch_pd,LOW);
}

void wifiSetup() {

    WifiPowerUp();
    
    delay(10/CLK_DIV);
    EspSerial.begin(9600*CLK_DIV);
    delay(10/CLK_DIV);

    //while(1) {
    #ifdef DBG_PRINT
    Serial.print(F("FW Version:"));
    Serial.println(wifi.getVersion().c_str());
    #endif
    //}
    
    if (wifi.setOprToStationSoftAP()) {
        #ifdef DBG_PRINT
        Serial.print(F("to station + softap ok\r\n"));
        #endif
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("to station + softap err\r\n"));
        #endif
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        #ifdef DBG_PRINT
        Serial.print(F("Join AP success\r\n"));

        Serial.print(F("IP:"));
        Serial.println( wifi.getLocalIP().c_str());  
        #endif     
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("Join AP failure\r\n"));
        #endif
    }
    
    if (wifi.disableMUX()) {
        #ifdef DBG_PRINT
        Serial.print(F("single ok\r\n"));
        #endif
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("single err\r\n"));
        #endif
    }

    #ifdef DBG_PRINT
    Serial.print(F("setup end\r\n"));
    #endif
    
    WifiPowerDown();

//    initializeFilters();
    
    //send_text_message("test");
}

