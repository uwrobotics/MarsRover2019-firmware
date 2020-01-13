#ifndef TUTORIAL_SERVO_H
#define TUTORIAL_SERVO_H

class TutorialServo {
public:
   // Constructor: Takes a servo pin name (ex. PA_1), and optionally a servo range
   // that has a default value of 180.0 degrees, a minimum pulsewidth of 1ms, and a     
   // maximum pulsewidth of 2ms.
   TutorialServo(PinName servoPin, float servoRangeInDegrees = 180.0, 
                 float minPulsewidthInMs = 1, float maxPulsewidthInMs = 2); 

   // Set servo range (ex. 270 deg)
   void setAngleRangeInDegrees(float degrees); 

   // Set servo pulse width range (ex. 1ms, 2ms
   void setPulsewidthRangeInMs(float minPulsewidthMs, float maxPulsewidthMs);

   // Set servo position (ex. 45 deg)
   void setPositionInDegrees(float degrees); 

    // I needed this function for the second part of the challenge
   // Get the servo range in degrees
   float getServoRangeInDegrees();

private:
   float  m_servoRangeInDegrees;
   PwmOut m_servoPwmOut;
   float m_minPulsewidthInMs;
   float m_maxPulsewidthInMs;
};

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

#endif