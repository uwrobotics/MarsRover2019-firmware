#ifndef TUTORIAL_SERVO_H
#define TUTORIAL_SERVO_H

#include "mbed.h"

class TutorialServo {

public:

   // Constructor: Takes a servo pin name (ex. PA_1), and optionally a servo range
   // that has a default value of 180.0 degrees, a minimum pulsewidth of 1ms, and a     
   // maximum pulsewidth of 2ms.
   TutorialServo(PinName servoPin, float servoRangeInDegrees = 180.0, 
      float minPulsewidthInMs = 0.000625, float maxPulsewidthInMs = 0.002625); 

   void setRangeInDegrees(float degrees);    // Set servo range (ex. 180 or 270 deg)
   void setPositionInDegrees(float degrees); // Set servo position (ex. 45 deg)
   void setPulsewidthRangeInMs(float minPulsewidthMs, float maxPulsewidthMs); // Set servo pulse width range (ex. 1ms, 2ms
   float getRangeInDegrees(); // accessor for range

private:

   float  m_servoRangeInDegrees;
   PwmOut m_servoPwmOut;
   float minPulsewidthMs;
   float maxPulsewidthMs;
};

#endif
