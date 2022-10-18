#include "SoilSensorService.h"
#include "Arduino.h"

double SoilSensorService::GetSensorReading(int gpio)
{
    return analogRead(gpio);
}

void SoilSensorService::ActivateSoilSensor(int gpio)
{
    digitalWrite(gpio, HIGH);
}

void SoilSensorService::DisableSoilSensor(int gpio)
{
    digitalWrite(gpio, LOW);
}