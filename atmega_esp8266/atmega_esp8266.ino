//Compile with Arduino Uno Board

#include <avr/sleep.h>
#include "ESP8266.h"
#include <avr/wdt.h>
#include <avr/power.h>
#include "aes256.h" //Include library files
#include "wifi_credentials.h"

#define SSID        WIFI_SSID
#define PASSWORD    WIFI_PASSWORD

#define HOST_NAME   "api.thingspeak.com"
#define HOST_PORT    80

#define TIME_BETWEEN_TEXT_MESSAGES 2 //in seconds
#define CLK_DIV 1
#define DBG_PRINT 1
#define PING_INTERVAL 900 //in watchdog intervals, max watchdog time = 8 sec
#define BUFFER_LENGTH 100
#define LOW_BATTERY_VOLTAGE_LEVEL 3100 //in mV

#define DUMP(str, i, buf, sz) { Serial.println(str); \
                               for(i=0; i<(sz); ++i) { if(buf[i]<0x10) Serial.print('0'); Serial.print(char(buf[i]), HEX); } \
                               Serial.println(); } //Help function for printing the Output

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(10,11); // RX, TX

ESP8266 wifi(EspSerial,Serial);

aes256_context ctxt;

char buffer[BUFFER_LENGTH];

const long InternalReferenceVoltage = 1082;//just this value to your board's specific internal BG voltage
const byte ch_pd = 8; //arduino pin number ESP8266 ch_pd is connected to
const byte LED = 13;
const byte ESP_RST_PIN = 6;
int count = PING_INTERVAL ;
byte water_detect = 0;

int getBandgap () 
  {
  // REFS0 : Selects AVcc external reference
  // MUX3 MUX2 MUX1 : Selects 1.1V (VBG)  
   ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
   ADCSRA |= bit( ADSC );  // start conversion
   while (ADCSRA & bit (ADSC))
     { }  // wait for conversion to complete
   int results = (((InternalReferenceVoltage * 1024) / ADC) + 5) / 10; 
   return results;
  } // end of getBandgap
  
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
   count--;
}  // end of WDT_vect

void pins_init()
{
  pinMode(ch_pd,OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(ESP_RST_PIN,OUTPUT);
}

void esp_cold_reset() {
  digitalWrite(ESP_RST_PIN,LOW);
  delay(2000);
  digitalWrite(ESP_RST_PIN,HIGH); 
}

void wake ()
{
  // cancel sleep as a precaution
  sleep_disable();
  // precautionary while we do other stuff
  detachInterrupt (0);
  water_detect = 1 ;
  //send_text_message("water detect interrupt fired");
}  // end of wake

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

//Return 1 on success, 0 on fail
int wifiSetup() {
    WifiPowerUp();

    delay(10/CLK_DIV);
    EspSerial.begin(9600*CLK_DIV);
    delay(10/CLK_DIV);

    #ifdef DBG_PRINT
    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());
    #endif
    
    if (wifi.setOprToStationSoftAP()) {
        #ifdef DBG_PRINT
        Serial.print(F("to station + softap ok\r\n"));
        #endif
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("to station + softap err\r\n"));
        #endif
        return 0;
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
        return 0;
    }
    
    if (wifi.disableMUX()) {
        #ifdef DBG_PRINT
        Serial.print(F("single ok\r\n"));
        #endif
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("single err\r\n"));
        #endif
        return 0;
    }

    #ifdef DBG_PRINT
    Serial.print(F("setup end\r\n"));
    Serial.flush();
    #endif
    
    WifiPowerDown();
    return 1 ;
//    initializeFilters();
    
    //send_text_message("test");
}

//Return 1 on success, 0 on fail
int send_text_message(String msg) {
   
   WifiPowerUp();
   delay(2000/CLK_DIV);

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        #ifdef DBG_PRINT
        Serial.print(F("create tcp ok\r\n"));
        #endif
    } else {
        #ifdef DBG_PRINT
        Serial.print(F("create tcp err\r\n"));
        #endif
        WifiPowerDown();
        return 0;
    }

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
    
    //delay(5000);
    //char start_char = '{';
    //wifi.recvRaw(buffer, BUFFER_LENGTH, start_char , 5000);

    wifi.recvEcho(3000);
    
    /*
    String data = wifi.recvString(5000);
    if (data.length() > 0) {
        #ifdef DBG_PRINT
        Serial.print(F("Received:["));
        Serial.println(data);
        Serial.flush();
        #endif
    }
   */

    #ifdef DBG_PRINT
     Serial.print(F("Received:["));
     Serial.println(buffer);
     Serial.flush();
    #endif
        
    delay(2000);
    
    water_detect = 0;
    WifiPowerDown();

    delay(TIME_BETWEEN_TEXT_MESSAGES*1000/CLK_DIV);

    return 1;
}

void setup () 
  {
  pins_init();
  
  esp_cold_reset();
  digitalWrite (2, HIGH);  // enable pull-up
  #ifdef DBG_PRINT
  Serial.begin(9600*CLK_DIV);
  #endif
  
  clock_prescale_set(clock_div_1);

  while(1) {
    if ( wifiSetup() )
      break ;
    else 
      esp_cold_reset() ;
  }

  Serial.print("VCC=");
  Serial.println(getBandgap());

  #ifdef DBG_PRINT
  Serial.println(F("setup done"));
  Serial.flush();
  #endif

  }  // end of setup

void loop () 
{
  
  //Serial.println("Begin loop");
  //delay(1000/CLK_DIV);
  if ( count < 1 ) {
    int vcc_voltage_mv = getBandgap()*10;
    String sensor_status = "sensor power vcc = " + String(vcc_voltage_mv) + " mv";

    //if (vcc_voltage_mv < LOW_BATTERY_VOLTAGE_LEVEL ) {
      while(1) {  
        if ( send_text_message(sensor_status) ) 
          break;
      }
    //} 
    
    count = PING_INTERVAL;
  }

  
  // disable ADC
  //ADCSRA = 0;  

    // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 

  //WDTCSR = bit (WDIE) ;    // set WDIE, and 16 mill seconds delay
  //WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);    // set WDIE, and 1 second delay
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  
  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();

  // Do not interrupt before we go to sleep, or the
  // ISR will detach interrupts and we won't wake.
  noInterrupts ();
  
  // will be called when pin D2 goes low  
  attachInterrupt (0, wake, FALLING);
  EIFR = bit (INTF0);  // clear flag for interrupt 0
 
  // turn off brown-out enable in software
  // BODS must be set to one and BODSE must be set to zero within four clock cycles
  //MCUCR = bit (BODS) | bit (BODSE);
  // The BODS bit is automatically cleared after three clock cycles
  //MCUCR = bit (BODS); 
  
  // We are guaranteed that the sleep_cpu call will be done
  // as the processor executes the next instruction after
  // interrupts are turned on.
  interrupts ();  // one cycle
  sleep_cpu ();   // one cycle

  //send_text_message("test message");
  delay(5000);

  if ( water_detect ) {
    while(1) { 
     if ( send_text_message("Family-Room: Fido barking his head off, try feeding him once in a while !") )
       break; 
    }
  }
  } // end of loop
