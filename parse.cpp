#include "json.h"

using namespace Json;

// Parse an object - create a new root, and populate.
Value Json::parse(const char *value)
{
    Value output;
    aJsonStringStream stream(value, NULL);
    stream.skip();
    stream.parseValue(&output, NULL);
    return output;
}

// Utility to jump whitespace and cr/lf
int aJsonStream::skip()
{
    int in = this->getch();
    while (in != EOF && (in <= 32))
        {
            in = this->getch();
        }
    if (in != EOF)
        {
            this->ungetch(in);
            return 0;
        }
    return EOF;
}

// Utility to flush our buffer in case it contains garbage
// since the parser will return the buffer untouched if it
// cannot understand it.
int aJsonStream::flush()
{
    int in = this->getch();
    while(in != EOF)
    {
        in = this->getch();
    }
    return EOF;
}


// Parser core - when encountering text, process appropriately.
int aJsonStream::parseValue(Value *item, char** filter)
{
    if (this->skip() == EOF)
        {
            return EOF;
        }
    //read the first byte from the stream
    int in = this->getch();
    if (in == EOF)
        {
            return EOF;
        }
    this->ungetch(in);
    if (in == '\"')
        {
            return this->parseString(item);
        }
    else if (in == '-' || (in >= '0' && in <= '9'))
        {
            return this->parseNumber(item);
        }
    else if (in == '[')
        {
            return this->parseArray(item, filter);
        }
    else if (in == '{')
        {
            return this->parseObject(item, filter);
        }
    //it can only be null, false or true
    else if (in == 'n')
        {
            //a buffer to read the value
            char buffer[] =
                { 0, 0, 0, 0 };
            if (this->readBytes((uint8_t*) buffer, 4) != 4)
                {
                    return EOF;
                }
            if (!strncmp(buffer, "null", 4))
                {
                    item->type = JSON_NULL;
                    return 0;
                }
            else
                {
                    return EOF;
                }
        }
    else if (in == 'f')
        {
            //a buffer to read the value
            char buffer[] =
                { 0, 0, 0, 0, 0 };
            if (this->readBytes((uint8_t*) buffer, 5) != 5)
                {
                    return EOF;
                }
            if (!strncmp(buffer, "false", 5))
                {
                    *item = false;
                    return 0;
                }
        }
    else if (in == 't')
        {
            //a buffer to read the value
            char buffer[] =
                { 0, 0, 0, 0 };
            if (this->readBytes((uint8_t*) buffer, 4) != 4)
                {
                    return EOF;
                }
            if (!strncmp(buffer, "true", 4))
                {
                    *item = true;
                    return 0;
                }
        }

    return EOF; // failure.
}

// Parse the input text into an unescaped cstring, and populate item.
int
aJsonStream::parseString(Value *item)
{
    //we do not need to skip here since the first byte should be '\"'
    int in = this->getch();
    if (in != '\"')
        return EOF; // not a string!
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    in = this->getch();
    int i = 0;
    while (in != EOF)
    {
        while (in != '\"' && in >= 32)
        {
            if (in != '\\')
            {
                buffer[i] = in;
            }
            else
            {
                in = this->getch();
                if (in == EOF)
                    return EOF;

                switch (in)
                {
                case '\\':
                    buffer[i] = '\\';
                    break;
                case '\"':
                    buffer[i] = '\"';
                    break;
                case '/':
                    buffer[i] = '/';
                    break;
                case 'b':
                    buffer[i] = '\b';
                    break;
                case 'f':
                    buffer[i] = '\f';
                    break;
                case 'n':
                    buffer[i] = '\n';
                    break;
                case 'r':
                    buffer[i] = '\r';
                    break;
                case 't':
                    buffer[i] = '\t';
                    break;
                default:
                    //we do not understand it so we skip it
                    break;
                }
            }
            i++;
            in = this->getch();
            if (in == EOF)
                return EOF;
        }
        //the string ends here
        *item = buffer;
        return 0;
    }
    //we should not be here but it is ok
    return 0;
}


// Build an array from input text.
int aJsonStream::parseArray(Value * item, char** filter)
{
    int in = this->getch();
    if (in != '[')
            return EOF; // not an array!

    *item = *(new Array());
    this->skip();
    in = this->getch();
    //check for empty array
    if (in == ']')
        {
            return 0; // empty array.
        }
    //now put back the last character
    this->ungetch(in);

    do
    {
        Value new_item;
        this->skip();
        if (this->parseValue(&new_item, filter))
        {
            return EOF;
        }
        item->asArray().append(new_item);
        this->skip();
        in = this->getch();
    } while(in == ',');
    if (in == ']')
    {
        return 0; // end of array
    }
    else
    {
        return EOF; // malformed.
    }
}


// Build an object from the text.
int aJsonStream::parseObject(Value *item, char** filter)
{
    int in = this->getch();
    if (in != '{')
    {
        return EOF; // not an object!
    }

    *item = *(new Object());
    this->skip();
    //check for an empty object
    in = this->getch();
    if (in == '}')
        {
            return 0; // empty object.
        }
    //preserve the char for the next parser
    this->ungetch(in);

    do
    {
        Value key_name;
        this->skip();
        if (this->parseString(&key_name) == EOF)
        {
            return EOF;
        }
        this->skip();

        in = this->getch();
        if (in != ':')
        {
            return EOF; // fail!
        }
        // skip any spacing, get the value.
        this->skip();
        Value value;
        if (this->parseValue(&value, filter) == EOF)
        {
            return EOF;
        }
        (item->asObject())[key_name.asString()] = value;
        key_name.free_parsed();
        this->skip();
        in = this->getch();
    } while (in == ',');

    if (in == '}')
    {
        return 0; // end of array
    }
    else
    {
        return EOF; // malformed.
    }
}
