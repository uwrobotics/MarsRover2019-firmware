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
    positionPID,
    velocityPID
} t_elevatorControlMode;

typedef struct{
    // Motor Configuration
    PinName elevatorMotor;
    float   maxMotorSpeed;

    // Motor Direction Configuration
    PinName elevatorMotorDir;

    // PID Configuration
    float PIDUpdateInterval;
    float elevatorPID_P;
    float elevatorPID_I;
    float elevatorPID_D;

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
        void  setElevatorVelocity( float setVelocity );
        float getPosition(); // Return encoder value (reference = top = 0 ?)
};

#endif // ELEVATOR_CONTROLLER_H