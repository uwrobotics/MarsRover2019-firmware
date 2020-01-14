#include <servo_control.h>
#include "mbed.h"

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
    float pulseWidth = 0.001 + 0.001*degrees;
    
    //need to compare in units of Ms not s
    if (pulseWidth*1000 > maxPulsewidthInMs || pulseWidth*1000 < minPulsewidthInMs)
    {
        return;
    }
    mservoPwmOut.pulsewidth(pulseWidth);
}

TutorialServo::setPulsewidthRangeInMs(float minPulsewidthms, float maxPulsewidthMs)
{
    minPulsewidthInMs = minPulsewidthms;
    maxPulsewidthInMs = maxPulsewidthMs;
}
