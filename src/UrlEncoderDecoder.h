#ifndef UrlEncoderDecoder_h
#define UrlEncoderDecoder_h
#include "Arduino.h"

class UrlEncoderDecoderService
{
    public:
       unsigned char h2int(char c);
       String urldecode(String str);
       String urlencode(String str);
};

#endif