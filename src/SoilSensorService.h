#ifndef SoilSensorService_h
#define SoilSesnsorService_h
#include "Arduino.h"

class SoilSensorService
{
    public:
        double GetSensorReading(int gpio);
        void ActivateSoilSensor(int gpio);
        void DisableSoilSensor(int gpio);
};

#endif