#ifndef SoilSensorService_h
#define SoilSesnsorService_h
#include "Arduino.h"

class SoilSensorService
{
    public:
        int GetSensorReading(int gpio);
};

#endif