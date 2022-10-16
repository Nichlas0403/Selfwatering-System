#include "WaterPumpService.h"
#include <Arduino.h>

void WaterPumpService::StartWaterPump(int gpio)
{
    digitalWrite(gpio, HIGH);
}

void WaterPumpService::StopWaterPump(int gpio)
{
    digitalWrite(gpio, LOW);
}