#include "ESP8266.h"
#include "wifi_credentials.h"

#define SSID        WIFI_SSID
#define PASSWORD    WIFI_PASSWORD

#define HOST_NAME   "api.thingspeak.com"
#define HOST_PORT    80

#define SOUND_SENSOR A0
#define LED 13      // the number of the LED pin
#define THRESHOLD_VALUE 15 //The threshold to turn the led on 400.00*5/1024 = 1.95v
#define NUM_SAMPLES 50
#define SAMPLE_INTERVAL 20
#define FILTER_SIZE 3
#define TIME_BETWEEN_TEXT_MESSAGES 10 //in seconds

#include <SoftwareSerial.h>
SoftwareSerial EspSerial(10,11); // RX, TX

ESP8266 wifi(EspSerial);

int ch_pd = 4;
   int sensorValue ;
   int previousSensorValue;
   
class WindowFilter {
  private:
    int filter[FILTER_SIZE];
    int wptr = 0;
    int overwritten_value ;
    int sum = 0; 
  public:  
    WindowFilter() {
      for(int i=0;i<FILTER_SIZE;i++)
        filter[i]=0;
    }

    void add(int sampled_value) {
      overwritten_value = filter[wptr];
      sum -= overwritten_value ;
      sum += sampled_value ;
      filter[wptr] = sampled_value ;
      incrementWptr();
    }

   int getOverwrittenValue() {
      return overwritten_value ;
   }

   int getAvg() {
      return sum/FILTER_SIZE;
    }
   
  private:
    void incrementWptr() {
      wptr++;
      if (wptr >= FILTER_SIZE)
        wptr = 0;
    }
};

//WindowFilter current;
//WindowFilter previous ;

void pins_init()
{
  pinMode(ch_pd,OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(SOUND_SENSOR, INPUT); 
}
void turnOnLED()
{
  digitalWrite(LED,HIGH);
}
void turnOffLED()
{
  digitalWrite(LED,LOW);
}

void setup(void)
{
    
    pins_init();
    
    Serial.begin(9600);
    Serial.print("setup begin\r\n");

    wifiSetup();

    sensorValue = getSoundSensorValue();
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
/*
void initializeFilters() {
    Serial.println("Intializing window filters ...");
    for(int i=0;i<FILTER_SIZE*2;i++) {
      int sensorAvg = getSoundSensorValue();
      current.add(sensorAvg) ;
      previous.add( current.getOverwrittenValue());
    }
    Serial.println("...Done");
}
*/

void WifiPowerUp() {
  Serial.println("** Power Up Wifi **");
  digitalWrite(ch_pd,HIGH);
}

void WifiPowerDown() {
  Serial.println("-- Power Down Wifi --");
  digitalWrite(ch_pd,LOW);
}

void send_text_message(String msg) {
   
   WifiPowerUp();
   delay(2000);

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    String preamble = "GET /apps/thinghttp/send_request?api_key=YNUF28WWU6S33RZH&number=5126570021&message=";
    String postamble = " HTTP/1.0\r\n\r\n";
    String GET_REQUEST = preamble + msg + postamble;

    Serial.print(GET_REQUEST);
    Serial.println("");
    Serial.print("REQUEST length = ");
    Serial.println(GET_REQUEST.length());
    
    wifi.send((const uint8_t*)GET_REQUEST.c_str(), GET_REQUEST.length());
    
    //delay(5000);
    
    String data = wifi.recvString(5000);
    if (data.length() > 0) {
        Serial.print("Received:[");
        Serial.println(data);
    }
    
    WifiPowerDown();

    delay(TIME_BETWEEN_TEXT_MESSAGES*1000);
    
    //initializeFilters();
  
}

int getSoundSensorValue() {
    //Average over NUM_SAMPLES , SAMPLE_INTERVAL ms apart, 
  int sum = 0;
  for(int i=0;i<NUM_SAMPLES;i++) {
    int sensorValue = analogRead(SOUND_SENSOR);//use A0 to read the electrical signal
    sum += sensorValue;
    delay(SAMPLE_INTERVAL);
  }
  int sensorAvg = sum/NUM_SAMPLES;
  return sensorAvg;
}

void loop(void)
{
   previousSensorValue = sensorValue ;
   sensorValue = getSoundSensorValue();
   //current.add(sensorAvg) ;
   //previous.add( current.getOverwrittenValue());

  //int currentAvg = current.getAvg();
  //int previousAvg = previous.getAvg();

  int diff = ((sensorValue-previousSensorValue)*100)/sensorValue ;

  /*
  Serial.print("sensorValue = ");
  Serial.print(sensorValue);
  Serial.print(", diff = ");
  Serial.println(diff);
  */
  
  /*
  Serial.print("Current = ");
  Serial.println(currentAvg);

  Serial.print("Previous = ");
  Serial.println(previousAvg);
  
  int difference = currentAvg - previousAvg ;
  Serial.print("Difference = ");
  Serial.println(difference);
   */
   
  if(diff > THRESHOLD_VALUE)
  {  
    turnOnLED();
    String msg = "Sound-alarm(" + String(THRESHOLD_VALUE) +")=" + diff ;
    Serial.println(msg);
    send_text_message(msg); 
  }
  turnOffLED();  
}
