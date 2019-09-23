#include "TutorialServo.h"
#include "mbed.h"
 
TutorialServo::TutorialServo(PinName servoPin, float servoRangeInDegrees, float minPW, float maxPW) : 
	m_servoPwmOut(servoPin), m_servoRangeInDegrees(servoRangeInDegrees), minPulsewidthMs(minPW), maxPulsewidthMs(maxPW) {}
 
void TutorialServo::setRangeInDegrees(float degrees) {
	// assume between 0 and 360
	if ((degrees < 360) && (degrees > 0))
    	m_servoRangeInDegrees = degrees;
}

void TutorialServo::setPositionInDegrees(float degrees) {
    //m_servoPwmOut.pulsewidth(0.000625 + 0.002*degrees/m_servoRangeInDegrees);
    // check if input is in range and positive
    if (degrees > m_servoRangeInDegrees)
    	degrees = m_servoRangeInDegrees;
    else if (degrees < 0)
    	degrees = 0;

    // set position using pwm
    m_servoPwmOut.pulsewidth(minPulsewidthMs + ( (maxPulsewidthMs - minPulsewidthMs)*degrees/m_servoRangeInDegrees ) );
}

float TutorialServo::getRangeInDegrees() {
    return m_servoRangeInDegrees;
}