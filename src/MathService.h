#ifndef MathService_h
#define MathService_h
#include "Arduino.h"

class MathService
{
    public:
        long ConvertMinutesToMillis(byte minutes);
        long ConvertSecondsToMillis(int seconds);
        double ConvertMillisToHours(long millis);
};

#endif