#include "MathService.h"
#include "Arduino.h"

long MathService::ConvertMinutesToMillis(byte minutes)
{
    return minutes / 60.000;
}

long MathService::ConvertSecondsToMillis(int seconds)
{
    return seconds / 1000;
}

int MathService::ConvertMillisToHours(long millis)
{
    return millis * E-7
}