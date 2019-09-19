#include "TutorialServo.h"
#include "mbed.h"
 
TutorialServo::TutorialServo(PinName servoPin, float servoRangeInDegrees) : 
	m_servoPwmOut(servoPin), m_servoRangeInDegrees(servoRangeInDegrees) {
    m_servoPwmOut = 0;
}
 
void TutorialServo::setRangeInDegrees(float degrees) {
	// assume between 0 and 360
	if ((degrees < 360) && (degrees > 0))
    	m_servoRangeInDegrees = degrees;
}

void TutorialServo::setPositionInDegrees(float degrees) {
    // check if input is in range and positive
    if (degrees > m_servoRangeInDegrees)
    	degrees = m_servoRangeInDegrees;
    else if (degrees < 0)
    	return

    // set position using pwm
    m_servoPwmOut.pulsewidth(0.001 + 0.001*degrees/m_servoRangeInDegrees);
}