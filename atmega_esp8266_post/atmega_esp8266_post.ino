#include <avr/sleep.h>
#include "ESP8266.h"
#include <avr/wdt.h>
#include <avr/power.h>
#include <AESLib.h>
#include <MeasureVcc.h>
#include <ArduinoJson.h>
#include "wifi_credentials.h"

#define DEVICE_ID 99
#define DEVICE_BANDGAP 1082

#define SSID        WIFI_SSID
#define PASSWORD    WIFI_PASSWORD

//#define HOST_NAME   "api.thingspeak.com"
//#define HOST_PORT    80

//#define HOST_NAME   "192.168.43.83"
//#define HOST_NAME   "192.168.1.15"
//#define HOST_PORT    3000
#define HOST_NAME   "h2ando.herokuapp.com"
#define HOST_PORT    80

#define TIME_BETWEEN_TEXT_MESSAGES 2 //in seconds
#define CLK_DIV 1
#define DBG_PRINT 1
//#define PING_INTERVAL 1440 //in watchdog intervals, max watchdog time = 8 sec
#define BUFFER_LENGTH 50
#define MAX_BODY_LENGTH 32 //Make it a multiple of 16, since AES does stuff in 16 byte blocks
#define SERIAL_RX_TIMEOUT 5000 //in milliseconds

#include "SoftwareSerial.h"
SoftwareSerial EspSerial(10,11); // RX, TX

ESP8266 wifi(EspSerial,Serial);

// uint8_t key[] = { //
//     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
//   };

char buffer[BUFFER_LENGTH];

const byte ch_pd = 8; //arduino pin number ESP8266 ch_pd is connected to
const byte LED = 13;
int count = 1 ;
const byte ESP_RST_PIN = 6;
char alarm_detect = 0;

void pins_init()
{
  pinMode(ch_pd,OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(ESP_RST_PIN,OUTPUT);
}

int test_post_request(String body) {

   int i ;
   StaticJsonBuffer<75> jsonBuffer;
   
   //uint8_t encrypted_body[MAX_BODY_LENGTH];

   //for(i=0;i<MAX_BODY_LENGTH;i++)
    //encrypted_body[i] = random(35,122);//Range of printable ascii characters, excluding '{' and '}'.
    //encrypted_body[i] = 65 ;//Range of printable ascii characters, excluding '{' and '}'.

   WifiPowerUp();
   delay(2000/CLK_DIV);

    create_tcp_conn();

    String preamble = "POST /postform HTTP/1.0\r\nhost:" + String(HOST_NAME) + "\r\n";
    //String header = "content-length:" + String(MAX_BODY_LENGTH) + "\r\n" ;
    String header = "content-length:" + String(body.length()) + "\r\n" ;
    String blank_line = "\r\n";

    //for(i=0;i<body.length();i++)
      //encrypted_body[i] = body.c_str()[i];

    //aes128_cbc_enc(key, key, encrypted_body, MAX_BODY_LENGTH);

    //String POST_REQUEST = preamble + header + blank_line + String((char *)encrypted_body) + blank_line + blank_line ;
    String POST_REQUEST = preamble + header + blank_line + body + blank_line + blank_line ;

    #ifdef DBG_PRINT
    Serial.print(POST_REQUEST);
    Serial.println("");
    Serial.print("REQUEST length = ");
    Serial.println(POST_REQUEST.length());
    #endif

    wifi.send((const uint8_t*)POST_REQUEST.c_str(), POST_REQUEST.length());

    //wifi.recvEcho(SERIAL_RX_TIMEOUT);
    wifi.recvRaw(buffer, BUFFER_LENGTH, '{', SERIAL_RX_TIMEOUT ) ;
    #ifdef DBG_PRINT
    Serial.println(buffer);
    #endif
    WifiPowerDown();

    JsonObject & root = jsonBuffer.parseObject(buffer);

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
    if (e == 0)
      return p;
    else
      return e;
}

float getSupplyVoltage() {
  int i;
  float voltage = 0;
  for (i=0;i<10;i++) {
    voltage += analogRead(5);
  }
  voltage /= 10.0 ;
  return voltage*3.3/1023 ;
}

void setup ()
  {
  analogReference(EXTERNAL);    
  pins_init();

  esp_cold_reset();

  digitalWrite (2, HIGH);  // enable pull-up
  #ifdef DBG_PRINT
  Serial.begin(9600*CLK_DIV);
  #endif

  Serial.print(F("VCC = "));
  Serial.println(getSupplyVoltage());
  
  clock_prescale_set(clock_div_1);
  pins_init();
  wifiSetup();

  Serial.print("VCC = ");
  Serial.println(getBandgap(DEVICE_BANDGAP));
    
  #ifdef DBG_PRINT
  Serial.println("setup done");
  #endif
}  // end of setup

int msg_id = 0;
void loop ()
{
   String msg ;

   if (count < 1) {
      msg_id++;
      msg = "{\"i\":" + String(DEVICE_ID) + ",\"m\":" + String(msg_id,DEC) + ",\"v\":" + String(getBandgap(DEVICE_BANDGAP),DEC) + ",\"a\":" + String(0) + "}" ;
      #ifdef DGB_PRINT
      Serial.println(msg);
      #endif
      count=test_post_request(msg);
      count = count/8;
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

  if ( alarm_detect ) {
      msg_id++;
      msg = "{\"i\":" + String(DEVICE_ID) + ",\"m\":" + String(msg_id,DEC) + ",\"v\":" + String(getBandgap(DEVICE_BANDGAP),DEC) + ",\"a\":" + String(1) + "}" ;
      Serial.println(msg);
      count=test_post_request(msg);
      //If post successful would have returned non-zero value.
      if (count > 0) alarm_detect = 0;
      count = count/8;
  }

} // end of loop
