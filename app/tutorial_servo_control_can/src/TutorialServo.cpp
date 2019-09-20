#include "TutorialServo.h"
#include "mbed.h"
 
TutorialServo::TutorialServo(PinName servoPin, float servoRangeInDegrees) : m_servoPwmOut(servoPin) {
    m_servoRangeInDegrees = servoRangeInDegrees;
    m_servoPwmOut.period(0.020); // Seconds
}
 
// What is this for?
void TutorialServo::setRangeInDegrees(float degrees) {
    if (degrees == 180.0 || degrees == 270.0)
        m_servoRangeInDegrees = degrees;
}

void TutorialServo::setPositionInDegrees(float degrees) {
    m_servoPwmOut.pulsewidth(0.001 + 0.001 * (degrees/m_servoRangeInDegrees));
}