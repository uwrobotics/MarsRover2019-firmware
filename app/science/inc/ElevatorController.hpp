#ifndef ELEVATOR_CONTROLLER_H
#define ELEVATOR_CONTROLLER_H
// Controller for the Elevator moving the Auger

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"

// Methods of control
typedef enum t_elevatorControlMode {
    motorSpeed,
    positionPID
} t_elevatorControlMode;

typedef struct{
    // Motor Configuration
    PinName elevatorMotor;
    float   maxMotorSpeed;

    // Motor Direction Configuration
    PinName elevatorMotorDir;

    // Encoder config
    PinName elevatorEncoder;

    // PID Configuration
    float PIDUpdateInterval;
    float positionPID_P;
    float positionPID_I;
    float positionPID_D;

    float EXTENTION_ENCODER_LIMIT; //Lower limit for encoder

} t_elevatorControllerConfig;

class ElevatorController{
    private:
        t_elevatorControlMode      m_elevatorControlMode;
        t_elevatorControllerConfig m_elevatorControllerConfig;
    public:
        ElevatorController( t_elevatorControllerConfig m_elevatorControllerConfig,
                            t_elevatorControlMode      m_elevatorControlMode );

        void  setControlMode( t_elevatorControlMode );
        void  setElevatorMotorSpeed( float setSpeed );
        void  setPosition( float position );
        void  maxLower(); // Wrapper for setPosition with lower encoder limit
        void  retract(); // Wrapper for setPosition with encoder = 0
        float getPosition(); // Return encoder value (reference = top = 0 ?)
};

#endif // ELEVATOR_CONTROLLER_H