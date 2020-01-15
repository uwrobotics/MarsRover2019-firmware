#include "servo_control.h"
#include "mbed.h"

TutorialServo::TutorialServo()
{
    m_servoRangeInDegrees = 270;
    maxPulsewidthMs = 2;
    minPulsewidthMs = 1;
    mservoPwmOut(PA_1);
}

TutorialServo::TutorialServo(PinName servoPin, Float servoRangeInDegrees, float minPulsewidthInMs, float maxPulsewidthInMs)
{
    m_servoRangeInDegrees = servoRangeInDegrees;
    m_maxPulsewidthMs = maxPulsewidthInMs;
    m_minPulsewidthMs = minPulsewidthInMs;
    mservoPwmOut(servoPin);
}

TutorialServo::setAngleRangeInDegrees(float degrees)
{
    m_servoRangeInDegrees = degrees;
}

TutorialServo::setPositionInDegrees(float degrees)
{
    if (degrees > m_servoRangeInDegrees)
    {
        //exit beforing attempting to set position greater than set max
        return;
    }
    //convert degrees into pulse width
    //the position is determined by the percentage that the value is out of 180 degrees eg. 90 is 1.5ms while 180 is 2ms
    float pulseWidth = m_minPulsewidthMs + (m_maxPulsewidthMs - m_minPulsewidthMs)*degrees/180;
    mservoPwmOut.pulsewidth(pulseWidth);
}

TutorialServo::setPulsewidthRangeInMs(float minPulsewidthMs, float maxPulsewidthMs)
{
    m_minPulsewidthMs = minPulsewidthms;
    m_maxPulsewidthMs = maxPulsewidthMs;
}
