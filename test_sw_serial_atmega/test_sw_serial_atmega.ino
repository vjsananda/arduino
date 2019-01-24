#include <avr/sleep.h>
#include "ESP8266.h"
#include "wifi_credentials.h"

#define SSID        WIFI_SSID
#define PASSWORD    WIFI_PASSWORD

#define HOST_NAME   "api.thingspeak.com"
#define HOST_PORT    80

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(10,11); // RX, TX

ESP8266 wifi(EspSerial);

const byte ch_pd = 9; //arduino pin number ESP8266 ch_pd is connected to
const byte LED = 13;

void pins_init()
{
  pinMode(ch_pd,OUTPUT);
  pinMode(LED, OUTPUT);
}

void wake ()
{
  // cancel sleep as a precaution
  sleep_disable();
  // precautionary while we do other stuff
  detachInterrupt (0);
}  // end of wake

void WifiPowerUp() {
  Serial.println("** Power Up Wifi **");
  digitalWrite(ch_pd,HIGH);
}

void WifiPowerDown() {
  Serial.println("-- Power Down Wifi --");
  digitalWrite(ch_pd,LOW);
}

void wifiSetup() {
      WifiPowerUp();

    delay(10);
    EspSerial.begin(9600);
    delay(10);
    
    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());

    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");

        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }
    
    Serial.print("setup end\r\n");
    WifiPowerDown();

//    initializeFilters();
  
    //send_text_message("test");
}
void setup () 
  {
  digitalWrite (2, HIGH);  // enable pull-up
  Serial.begin(9600);
  pins_init();
  wifiSetup();
  Serial.println("setup done");
  }  // end of setup

void loop () 
{
  for(int i=0;i<4;i++) {
    Serial.println("Interrupt triggered");
    digitalWrite (LED, HIGH);
    delay (50);
    digitalWrite (LED, LOW);
    delay (50);
  }
  pinMode (LED, INPUT);
  
  // disable ADC
  ADCSRA = 0;  
  
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
  MCUCR = bit (BODS) | bit (BODSE);
  // The BODS bit is automatically cleared after three clock cycles
  MCUCR = bit (BODS); 
  
  // We are guaranteed that the sleep_cpu call will be done
  // as the processor executes the next instruction after
  // interrupts are turned on.
  interrupts ();  // one cycle
  sleep_cpu ();   // one cycle

  } // end of loop
