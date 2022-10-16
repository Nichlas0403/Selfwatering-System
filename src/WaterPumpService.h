#ifndef WaterPumpService_h
#define WaterPumpService_h
#include <Arduino.h>

class WaterPumpService
{
    public:
        void StartWaterPump(int gpio);
        void StopWaterPump(int gpio);
};

#endif