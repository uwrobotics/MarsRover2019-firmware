
#include "MoistureSensor.h"

MoistureSensor::MoistureSensor(PinName dataPin, PinName powerPin) : m_analogIn(dataPin), m_power(powerPin) {}

void MoistureSensor::powerOn() {
    m_power = 1;
}

float MoistureSensor::readPercentage() {
    return m_analogIn.read();
}

void MoistureSensor::powerOff() {
    m_power = 0;
}
