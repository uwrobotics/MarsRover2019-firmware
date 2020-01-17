#include "mbed.h"
#include "rover_config.h"
#include <TutorialServo.h>

TutorialServo::TutorialServo(PinName servoPin, float servoRangeInDegrees, float minPulsewidthInMs, float maxPulsewidthInMs) 
                            : m_servoPwmOut(servoPin){
    // Set the range the servo can reach
    setAngleRangeInDegrees(servoRangeInDegrees);
    // Set the PWM range
    setPulsewidthRangeInMs(minPulsewidthInMs, maxPulsewidthInMs);
    // Set the frequency of Servo object to 50Hz.
    m_servoPwmOut.period(0.02);
}

void TutorialServo::setAngleRangeInDegrees(float degrees){
    m_servoRangeInDegrees = degrees;
}

void TutorialServo::setPulsewidthRangeInMs(float minPulsewidthMs, float maxPulsewidthMs){
    m_minPulsewidthInMs = minPulsewidthMs;
    m_maxPulsewidthInMs = maxPulsewidthMs;
}

void TutorialServo::setPositionInDegrees(float degrees){
    // map the value in degrees to a PWM value that makes sense for the parameters the user specified.
    // Assume the range in degrees always starts at 0
    float previousRange = m_servoRangeInDegrees;
    float curRange = m_maxPulsewidthInMs - m_minPulsewidthInMs;
    float pwm = degrees* (curRange/previousRange) + m_minPulsewidthInMs;
    // Position the Servo
    m_servoPwmOut.pulsewidth(pwm);
}

float TutorialServo::getServoRangeInDegrees(){
    return m_servoRangeInDegrees;
}