
#include <ArduinoJson.h>

void setup() {

  // put your setup code here, to run once:
  StaticJsonBuffer<200> jsonBuffer;

  Serial.begin(115200);

  JsonObject& root = jsonBuffer.createObject();
  root["sensor"] = "gps";
  root["time"] = 1351824120;

  JsonArray& data = root.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

  String JsonString ;

  root.printTo(JsonString);

  Serial.println(JsonString);

}

void loop() {
  // put your main code here, to run repeatedly:

}
