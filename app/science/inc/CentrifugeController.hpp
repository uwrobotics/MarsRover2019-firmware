#ifndef CENTRIFUGE_CONTROLLER_H
#define CENTRIFUGE_CONTROLLER_H
// Controller for the Centrifuge/Turntable

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"

// Methods of control
typedef enum t_centrifugeControlMode {
    motorSpeed,
    positionPID
} t_centrifugeControlMode;

typedef struct{
    // Motor Configuration
    PinName centrifugeMotor;

    // Motor Direction Configuration
    PinName centrifugeMotorDir;

    // Encoder Configuration
    PinName centrifugeEncoder;

    // PID Configuration
    float PIDUpdateInterval;
    float positionPID_P;
    float positionPID_I;
    float positionPID_D;

    // Max and min PWM signals for motor unit speed
    float PWM_MIN_SIGNAL = -1.0;
    float PWM_MAX_SIGNAL = 1.0;

} t_centrifugeControllerConfig;

class CentrifugeController{
    private:
        t_centrifugeControlMode      m_centrifugeControlMode;
        t_centrifugeControllerConfig m_centrifugeControllerConfig;
        const int                    LIMIT_SWITCH_ANGLE_OFFSET = 15; // Value in degrees
    public:
        CentrifugeController( t_centrifugeControllerConfig m_centrifugeControllerConfig,
                              t_centrifugeControlMode      m_centrifugeControlMode );

        void  setControlMode( t_centrifugeControlMode );
        void  setCentrifugeMotorSpeed( float setSpeed );

        // tube_num within range [0-11]
        // Places tube of # tube_num under the Auger output mouth
        void  setCentrifugePosition( int tube_num );

        float getAngle(); // Returns angle tube_0 is from under Auger mouth

        void resetToTube0(); // Restores initial position of centrifuge (ie. tube_0 under Auger mouth)
};

#endif // CENTRIFUGE_CONTROLLER_H