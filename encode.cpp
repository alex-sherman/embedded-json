#include "json.h"

#define FLOAT_PRECISION 5

int printFloat(double d, Print *print);
int printStringPtr(const char *str, Print *print);
int printArray(Json::Array &value, Print *print);
int printObject(Json::Object &value, Print *print);

class JsonDumper : public Print {
public:
  char *buffer;
  size_t size;
  JsonDumper(char *buffer, size_t size) {
    this->size = size;
    this->buffer = buffer;
  }
  virtual size_t write(uint8_t ch) {
    size_t ret = -1;
    if(size > 0) {
      ret = 1;
      *buffer = ch;
      size--;
      buffer++;
    }
    return ret;
  }
};

int Json::dump(Json::Value value, char* out, size_t size) {
  JsonDumper stream(out, size);
  int length = Json::print(value, stream);
  if(length > 0) out[length] = 0;
  return length;
}


int Json::println(Value v, Print &p) { print(v, p); p.print('\n'); }

// Render a value to text.
int Json::print(Value item, Print &print)
{
  int result = 0;
  switch (item.type)
  {
    case JSON_NULL:
      result = print.print("null");
      break;
    case JSON_BOOLEAN:
      if(item.asBool()){
        result = print.print("true");
      }
      else{
        result = print.print("false");
      }
      break;
    case JSON_INT:
      result = print.print(item.asInt());
      break;
    case JSON_FLOAT:
      result = printFloat(item.asFloat(), &print);
      break;
    case JSON_STRING:
      result = printStringPtr(item.asString(), &print);
      break;
    case JSON_ARRAY:
      result = printArray(item.asArray(), &print);
      break;
    case JSON_OBJECT:
      result = printObject(item.asObject(), &print);
      break;
  }
  return result;
}

int printFloat(double d, Print *print)
{
  int result = 0;
  if (d<0.0) {
    print->print("-");
    d=-d;
  }
  //print the integer part
  unsigned long integer_number = (unsigned long)d;
  result += print->print(integer_number, DEC);
  result += print->print(".");
  //print the fractional part
  double fractional_part = d - ((double)integer_number);
  //we do a do-while since we want to print at least one zero
  //we just support a certain number of digits after the '.'
  int n = FLOAT_PRECISION;
  fractional_part += 0.5/pow(10.0, FLOAT_PRECISION);
  do {
    //make the first digit non fractional(shift it before the '.'
    fractional_part *= 10.0;
    //create an int out of it
    unsigned int digit = (unsigned int) fractional_part;
    //print it
    result += print->print(digit, DEC);
    //remove it from the number
    fractional_part -= (double)digit;
    n--;
  } while ((fractional_part!=0) && (n>0));
  return result;
}

// Render the cstring provided to an escaped version that can be printed.
int printStringPtr(const char *str, Print *print)
{
  int result = 0;
  result += print->print("\"");
  char* ptr = (char*) str;
  if (ptr != NULL)
  {
    while (*ptr != 0)
    {
      if ((unsigned char) *ptr > 31 && *ptr != '\"' && *ptr != '\\')
      {
        result += print->print(*ptr);
        ptr++;
      }
      else
      {
        result += 2;
        print->print('\\');
        switch (*ptr++)
        {
        case '\\':
          print->print('\\');
          break;
        case '\"':
          print->print('\"');
          break;
        case '/':
          print->print('/');
          break;
        case '\b':
          print->print('b');
          break;
        case '\f':
          print->print('f');
          break;
        case '\n':
          print->print('n');
          break;
        case '\r':
          print->print('r');
          break;
        case '\t':
          print->print('t');
          break;
        default:
          break; // eviscerate with prejudice.
        }
      }

    }
  }
  result += print->print('\"');
  return result;
}
// Render an array to text
int printArray(Json::Array &value, Print *print)
{
  int result = 0;
  result += print->print('[');
  for(int i = 0; i < value.size(); i++) {
    result += Json::print(value[i], *print);
    if(i < value.size() - 1)
      result += print->print(',');
  }
  result += print->print(']');
  return result;
}

// Render an object to text.
int printObject(Json::Object &value, Print *print)
{
  int result = 0;
  result += print->print('{');
  for(int i = 0; i < value.size(); i++) {
    KeyValuePair<Json::Value> kvp = value.get(i);
    if(kvp.value.isInvalid())
      continue;
    if(i > 0)
      result += print->print(',');
    result += printStringPtr(kvp.key, print);
    result += print->print(':');
    result += Json::print(kvp.value, *print);
  }
  result += print->print("}");
  return result;
}