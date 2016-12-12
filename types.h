#pragma once

#include "amap.h"
#include <string.h>

#define JSON_NULL 0
#define JSON_BOOLEAN 1
#define JSON_INT 2
#define JSON_FLOAT 3
#define JSON_STRING 4
#define JSON_ARRAY 5
#define JSON_OBJECT 6
#define JSON_INVALID 255

namespace Json {

    class Value;

    class Object : public AMap<Value> {
    public:
        Object(Object &source) : AMap<Value>(source) { };
        Object() { };
        Object* clone();
        ~Object();
    protected:
        virtual Value default_init();
    private:
        Object(const Object&);
    };
    
    class Array : public AList<Value> {
    public:
        Array(Array &source) : AList<Value>(source) { };
        Array() { };
        Array* clone();
        ~Array();
    private:
        Array(const Array&);
    };

    class Value {
    public:
        Value(bool b) { type = JSON_BOOLEAN; valuebool = b; }
        Value(int i) { type = JSON_INT; valueint = i; }
        Value(uint i) { type = JSON_INT; valueint = i; }
        Value(float i) { type = JSON_FLOAT; valuefloat = i; }
        Value(const char *s) : Value((char*)s) { }
        Value(char *s) {
            type = JSON_STRING;
            char * buf = (char *)malloc(strlen(s) + 1);
#ifdef SMALLOC_DEBUG
            Serial.print("String malloc: ");
            Serial.println(int(buf));
#endif
            memcpy(buf, s, strlen(s) + 1);
            valuestring = buf;
        }
        Value(String s) {
            type = JSON_STRING;
            char * buf = (char *)malloc(s.length() + 1);
            buf[s.length()] = 0;
            s.toCharArray(buf, s.length() + 1);
            valuestring = buf;
        }
        Value(Array &a) : Value(&a) { }
        Value(Array *a) { type = JSON_ARRAY; valuearray = a; }
        Value(Object &o) : Value(&o) { }
        Value(Object *o) { type = JSON_OBJECT; valueobject = o; }
        Value() { type = JSON_NULL; }
        bool isBool() { return type == JSON_BOOLEAN; }
        int asBool() { return type == JSON_BOOLEAN ? valuebool : 0; }
        bool isFloat() { return type == JSON_FLOAT; }
        float asFloat() { return type == JSON_FLOAT ? valuefloat : 0.0f; }
        bool isDouble() { return type == JSON_FLOAT; }
        float asDouble() { return type == JSON_FLOAT ? valuefloat : 0.0; }
        bool isInt() { return type == JSON_INT; }
        int asInt() { return type == JSON_INT ? valueint : 0; }
        bool isString() { return type == JSON_STRING; }
        const char* asString() { return type == JSON_STRING ? valuestring : NULL; }
        bool isObject() { return type == JSON_OBJECT; }
        Object &asObject() { return *valueobject; }
        bool isArray() { return type == JSON_ARRAY; }
        Array &asArray() { return *valuearray; }
        bool isNull() { return type == JSON_NULL; }
        bool isInvalid() { return type == JSON_INVALID; }
        static Value invalid() {
            Value output;
            output.type = JSON_INVALID;
            return output;
        }
        void free_parsed() {
            switch(type) {
                case JSON_STRING:
#ifdef SMALLOC_DEBUG
                    Serial.print("String free: ");
                    Serial.println(int(valuestring));
#endif
                    delete (char*)valuestring;
                    break;
                case JSON_ARRAY:
                    delete valuearray;
                    break;
                case JSON_OBJECT:
                    delete valueobject;
                    break;
                default:
                    break;
            }
        }
        char type;
    private:
        union {
            double valuefloat;     // used for double and float
            int valueint;    // used for bool, char, short, int and longs
            const char* valuestring;  // asString can be null
            bool valuebool;
            Object *valueobject;    // asArray cannot be null
            Array *valuearray;
        };
        Value(char type) { this->type = type; }
    };
}