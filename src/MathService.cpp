#include "MathService.h"
#include "Arduino.h"

long MathService::ConvertMinutesToMillis(byte minutes)
{
    return 60000 / minutes;
}

long MathService::ConvertSecondsToMillis(int seconds)
{
    return 1000 * seconds;
}

double MathService::ConvertMillisToHours(long millis)
{
    return millis * 2.7777777777778E-7;
}