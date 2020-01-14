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
    //the position is determined by the percentage that the value is out of 180 degrees eg. 90 is 1.5ms while 180 is 2ms
    float pulseWidth = 0.001 + 0.001*degrees/180;
    
    //need to compare in units of Ms not s
    if (pulseWidth*1000 > maxPulsewidthInMs || pulseWidth*1000 < minPulsewidthInMs)
    {
        return;
    }
    mservoPwmOut.pulsewidth(pulseWidth);
}

TutorialServo::setPulsewidthRangeInMs(float minPulsewidthMs, float maxPulsewidthMs)
{
    minPulsewidthInMs = minPulsewidthMs;
    maxPulsewidthInMs = maxPulsewidthMs;
}
