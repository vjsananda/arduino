

//flag for saving data
bool shouldSaveConfig = false;
/*
#include <Ticker.h>
Ticker flipper;

#define BLINK_RATE_FAST 1
#define BLINK_RATE_SLOW 0

void flip() {
  digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));  
}

void ledBlinkOn(int rate ) {
  if ( rate == BLINK_RATE_FAST )
   flipper.attach(0.2,flip);
  else
   flipper.attach(0.5,flip);
}

void ledBlinkOff() {
  flipper.detach();
}
*/

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//Read saved parameters (sensor_id and iv_password) from Filesystem.
void readParam() {

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(sensor_id, json["sensor_id"]);
          strcpy(iv_password, json["iv_password"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void configureWiFi() {

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_sensor_id("sensor_id", "sensor id", sensor_id, 30);
  WiFiManagerParameter custom_iv_password("iv_password", "encrypt password", iv_password, 30);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  wifiManager.addParameter(&custom_sensor_id);
  wifiManager.addParameter(&custom_iv_password);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //ledBlinkOn(BLINK_RATE_SLOW);
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect and hit timeout");
    //ledBlinkOff();
    delay(3000);
    
    //reset and try again, or maybe put it to deep sleep
    ESP.deepSleep(sleepTimeS * 1000000);
    //ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  //ledBlinkOn(BLINK_RATE_FAST);
  delay(3000);
  //ledBlinkOff();

  //read updated parameters
  strcpy(sensor_id, custom_sensor_id.getValue());
  strcpy(iv_password, custom_iv_password.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["sensor_id"] = sensor_id;
    json["iv_password"] = iv_password;

    if (SPIFFS.begin()) {
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }
      else {
        json.printTo(Serial);
        json.printTo(configFile);
        configFile.close();
        //end save
      }
    }
    else {
      Serial.println("failed to mount SPIFFs");
    }
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

