#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H
// Controller for the Auger

#include "mbed.h"
#include "PinNames.h"

class MoistureSensor {

public:

    MoistureSensor(PinName dataPin, PinName powerPin);

    void powerOn(void);

    float readPercentage(void);

    void powerOff(void);

private:

    AnalogIn m_analogIn;
    DigitalOut m_power;

};


#endif // MOISTURE_SENSOR_H