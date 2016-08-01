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

#pragma once

#include <Print.h>
#include <Stream.h>
#include <Client.h>
#include <Arduino.h>  // To get access to the Arduino millis() function
#include "types.h"

#ifndef EOF
#define EOF -1
#endif

#define PRINT_BUFFER_LEN 256

namespace Json {

	Value parse(char*);
	int dump(Json::Value value, char* out, size_t size);
	int print(Value, Print&);
	int println(Value v, Print& p);
	int measure(Value);

	/* aJsonStream is stream representation of aJson for its internal use;
	 * it is meant to abstract out differences between Stream (e.g. serial
	 * stream) and Client (which may or may not be connected) or provide even
	 * stream-ish interface to string buffers. */
	class aJsonStream : public Print {
	public:
		aJsonStream(Stream *stream_)
			: stream_obj(stream_), bucket(EOF)
			{}
		/* Use this to check if more data is available, as aJsonStream
		 * can read some more data than really consumed and automatically
		 * skips separating whitespace if you use this method. */
		virtual bool available();

		int parseNumber(Value *);
		int parseString(Value *);

		int skip();
		int flush();

		int parseValue(Value *, char** filter);

		int parseArray(Value *, char** filter);

		int parseObject(Value *, char** filter);

	protected:
		/* Blocking load of character, returning EOF if the stream
		 * is exhausted. */
		/* Base implementation just looks at bucket, returns EOF
		 * otherwise; descendats take care of the real reading. */
		virtual int getch();
		virtual size_t readBytes(uint8_t *buffer, size_t len);
		/* Return the character back to the front of the stream
		 * after loading it with getch(). Only returning a single
		 * character is supported. */
		virtual void ungetch(char ch);

		/* Inherited from class Print. */
		virtual size_t write(uint8_t ch);

		/* stream attribute is used only from virtual functions,
		 * therefore an object inheriting aJsonStream may avoid
		 * using streams completely. */
		Stream *stream_obj;
		/* Use this accessor for stream retrieval; some subclasses
		 * may use their own stream subclass. */
		virtual inline Stream *stream() { return stream_obj; }

		/* bucket is EOF by default. Otherwise, it is a character
		 * to be returned by next getch() - returned by a call
		 * to ungetch(). */
		int bucket;
	};

	/* JSON stream that consumes data from a connection (usually
	 * Ethernet client) until the connection is closed. */
	class aJsonClientStream : public aJsonStream {
	public:
		aJsonClientStream(Client *stream_)
			: aJsonStream(NULL), client_obj(stream_)
			{}

	private:
		virtual int getch();

		Client *client_obj;
		virtual inline Client *stream() { return client_obj; }
	};

	/* JSON stream that is bound to input and output string buffer. This is
	 * for internal usage by string-based aJsonClass methods. */
	/* TODO: Elastic output buffer support. */
	class aJsonStringStream : public aJsonStream {
	public:
		/* Either of inbuf, outbuf can be NULL if you do not care about
		 * particular I/O direction. */
		aJsonStringStream(char *inbuf_, char *outbuf_ = NULL, size_t outbuf_len_ = 0)
			: aJsonStream(NULL), inbuf(inbuf_), outbuf(outbuf_), outbuf_len(outbuf_len_)
		{
			inbuf_len = inbuf ? strlen(inbuf) : 0;
		}

		virtual bool available();

	private:
		virtual int getch();
		virtual size_t write(uint8_t ch);

		char *inbuf, *outbuf;
		size_t inbuf_len, outbuf_len;
	};
}