#include <servo_control.h>
#include "mbed.h"

TutorialServo::TutorialServo()
{
    m_servoRangeInDegrees = 270;
    m_maxPulsewidthMs = 2;
    m_minPulsewidthMs = 1;
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
    //convert degree percentage into pulse width
    float pulseWidth = m_minPulsewidthMs + (m_maxPulsewidthMs - m_minPulsewidthMs)*degrees;
    mservoPwmOut.pulsewidth(pulseWidth);
}

TutorialServo::setPulsewidthRangeInMs(float minPulsewidthms, float maxPulsewidthMs)
{
    m_minPulsewidthMs = minPulsewidthms;
    m_maxPulsewidthMs = maxPulsewidthMs;
}
