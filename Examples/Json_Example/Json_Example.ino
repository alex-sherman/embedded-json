#include <json.h>

void setup() {
    Serial.begin(115200);
    Json::Value obj = new Json::Object();
    obj.asObject()["key"] = new Json::Array();
    Json::Array &array = obj.asObject()["key"].asArray();
    array.append(1);
    array.append("2");
    Json::Object &value3 = *new Json::Object();
    value3["3"] = true;
    array.append(value3);
    Json::println(obj, Serial);
    obj.free_parsed();
}

void loop() {
    
}