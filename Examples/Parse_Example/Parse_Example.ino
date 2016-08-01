#include <json.h>

void setup() {
    Serial.begin(115200);
    Json::Object &parsed = Json::parse("{\"key\":[1,\"2\",{\"3\":true}]}").asObject();
    parsed["key"].asArray()[2].asObject()["3"] = false;
    Json::println(parsed, Serial);
    delete &parsed;
}

void loop() {
    
}