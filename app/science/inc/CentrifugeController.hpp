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
    PinName     centrifugeMotor;
    const float maxMotorSpeed = 200; //In RPM, need to be rad/s?

    // Motor Direction Configuration
    PinName centrifugeMotorDir;

    // PID Configuration
    float PIDUpdateInterval;
    float centrifugePID_P;
    float centrifugePID_I;
    float centrifugePID_D;

} t_centrifugeControllerConfig;

class CentrifugeController{
    private:
        t_centrifugeControlMode      m_centrifugeControlMode;
        t_centrifugeControllerConfig m_centrifugeControllerConfig;
        const int                    LIMIT_SWITCH_ANGLE_OFFSET = 15;
    public:
        CentrifugeController( t_centrifugeControllerConfig m_centrifugeControllerConfig,
                              t_centrifugeControlMode      m_centrifugeControlMode );

        void  setControlMode( t_centrifugeControlMode );
        void  setCentrifugeMotorSpeed( float setSpeed );
        float getAngle(); // Returns angle tube_0 is from under Auger mouth

        void Reset(); // Restores initial position of centrifuge (ie. tube_0 under Auger mouth)
};

#endif // CENTRIFUGE_CONTROLLER_H