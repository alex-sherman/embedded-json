/*
 Copyright (c) 2001, Interactive Matter, Marcus Nowotny

 Based on the cJSON Library, Copyright (C) 2009 Dave Gamble

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

// aJSON
// aJson Library for Arduino.
// This library is suited for Atmega328 based Arduinos.
// The RAM on ATmega168 based Arduinos is too limited

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <ctype.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif
#include "json.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/
//Default buffer sizes - buffers get initialized and grow acc to that size
#define BUFFER_DEFAULT_SIZE 4

//how much digits after . for float
#define FLOAT_PRECISION 5

using namespace Json;

Object *Object::clone() {
    Object *copy = new Object(*this);
    for(auto &kvp : *this) {
        if(kvp.value.isArray())
            kvp.value = *kvp.value.asArray().clone();
        if(kvp.value.isObject())
            kvp.value = *kvp.value.asObject().clone();
        if(kvp.value.isString())
            kvp.value = kvp.value.asString();
    }
    return copy;
}

Array *Array::clone() {
    Array *copy = new Array(*this);
    for(auto &kvp : *this) {
        if(kvp.isArray())
            kvp = *kvp.asArray().clone();
        if(kvp.isObject())
            kvp = *kvp.asObject().clone();
        if(kvp.isString())
            kvp = kvp.asString();
    }
    return copy;
}
Object::~Object() {
    for(auto kvp : (*this)) {
            kvp.value.free_parsed();
    }
}
Array::~Array() {
    for(auto kvp : (*this)) {
            kvp.free_parsed();
    }
}

class JsonMeasurer : public Print {
public:
    size_t write(uint8_t ch) { _length += 1; return 1; }
    size_t length() { return _length; }
private:
    size_t _length;
};

int Json::measure(Value value) {
    JsonMeasurer m;
    Json::print(value, m);
}

bool
aJsonStream::available()
{
    if (bucket != EOF)
        return true;
    while (stream()->available())
        {
            /* Make an effort to skip whitespace. */
            int ch = this->getch();
            if (ch > 32)
             {
     this->ungetch(ch);
     return true;
             }
        }
    return false;
}

int
aJsonStream::getch()
{
    if (bucket != EOF)
        {
            int ret = bucket;
            bucket = EOF;
            return ret;
        }
    // In case input was malformed - can happen, this is the
    // real world, we can end up in a situation where the parser
    // would expect another character and end up stuck on
    // stream()->available() forever, hence the 500ms timeout.
    unsigned long i= millis()+500;
    while ((!stream()->available()) && (millis() < i)) /* spin with a timeout*/;
    return stream()->read();
}

void
aJsonStream::ungetch(char ch)
{
    bucket = ch;
}

size_t
aJsonStream::write(uint8_t ch)
{
    return stream()->write(ch);
}

size_t
aJsonStream::readBytes(uint8_t *buffer, size_t len)
{
    for (size_t i = 0; i < len; i++)
        {
            int ch = this->getch();
            if (ch == EOF)
    {
        return i;
    }
            buffer[i] = ch;
        }
    return len;
}


int
aJsonClientStream::getch()
{
    if (bucket != EOF)
        {
            int ret = bucket;
            bucket = EOF;
            return ret;
        }
    while (!stream()->available() && stream()->connected()) /* spin */;
    if (!stream()->available()) // therefore, !stream()->connected()
        {
            stream()->stop();
            return EOF;
        }
    return stream()->read();
}

bool
aJsonStringStream::available()
{
    if (bucket != EOF)
        return true;
    return inbuf_len > 0;
}

int
aJsonStringStream::getch()
{
    if (bucket != EOF)
    {
        int ret = bucket;
        bucket = EOF;
        return ret;
    }
    if (!inbuf || !inbuf_len)
    {
        return EOF;
    }
    char ch = *inbuf++;
    inbuf_len--;
    return ch;
}

size_t
aJsonStringStream::write(uint8_t ch)
{
    if (!outbuf || outbuf_len <= 1)
    {
        return 0;
    }
    *outbuf++ = ch; outbuf_len--;
    *outbuf = 0;
    return 1;
}

// Parse the input text to generate a number, and populate the result into item.
int aJsonStream::parseNumber(Value *item)
{
    int i = 0;
    int sign = 1;

    int in = this->getch();
    if (in == EOF)
    {
        return EOF;
    }
    // It is easier to decode ourselves than to use sscnaf,
    // since so we can easier decide between int & double
    if (in == '-')
    {
        //it is a negative number
        sign = -1;
        in = this->getch();
        if (in == EOF)
        {
            return EOF;
        }
    }
    if (in >= '0' && in <= '9') {
        do
        {
            i = (i * 10) + (in - '0');
            in = this->getch();
        }
        while (in >= '0' && in <= '9'); // Number?
    }
    //end of integer part � or isn't it?
    if (!(in == '.' || in == 'e' || in == 'E'))
    {
        *item = Value(i * (int) sign);
    }
    //ok it seems to be a double
    else
    {
        double n = (double) i;
        int scale = 0;
        int subscale = 0;
        char signsubscale = 1;
        if (in == '.')
        {
            in = this->getch();
            do
            {
                n = (n * 10.0) + (in - '0'), scale--;
                in = this->getch();
            }
            while (in >= '0' && in <= '9');
        } // Fractional part?
        if (in == 'e' || in == 'E') // Exponent?
        {
            in = this->getch();
            if (in == '+')
            {
                in = this->getch();
            }
            else if (in == '-')
            {
                signsubscale = -1;
                in = this->getch();
            }
            while (in >= '0' && in <= '9')
            {
                subscale = (subscale * 10) + (in - '0'); // Number?
                in = this->getch();
            }
        }

        n = sign * n * pow(10.0, ((double) scale + (double) subscale
                * (double) signsubscale)); // number = +/- number.fraction * 10^+/- exponent
        *item = Value(float(n));
    }
    //preserve the last character for the next routine
    this->ungetch(in);
    return 0;
}