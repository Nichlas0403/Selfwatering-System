#include "SoilSensorService.h"
#include "Arduino.h"

int SoilSensorService::GetSensorReading(int gpio)
{
    return 42;
    //return analogRead(gpio);
}