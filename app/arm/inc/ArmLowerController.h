#ifndef ARM_LOWER_CONTROLLER_H
#define ARM_LOWER_CONTROLLER_H

/* Controller for the arm base, shoulder and elbow
 */

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"

// TYPES

typedef enum t_controlMode {
    motorSpeed,
    velocityPID,
    positionPID

} t_controlMode;

typedef enum t_joint {
    baseJoint,
    elbowJoint,
    shoulderJoint

} t_joint;

typedef struct {
    // Motor config
    PinName baseMotor,
    PinName shoulderMotor,
    PinName elbowMotor,
    float maxMotorSpeed,

    // Motor direction config
    PinName baseMotorDir,
    PinName shoulderMotorDir,
    PinName elbowMotorDir,

    // Joint encoder config
    PinName baseEncoder,
    PinName shoulderEncoder,
    PinName elbowEncoder,

    // PID config
    float PIDUpdateInterval,
    float basePID_P,
    float basePID_I,
    float basePID_D,
    float shoulderPID_P,
    float shoulderPID_I,
    float shoulderPID_D,
    float elbowPID_P,
    float elbowPID_I,
    float elbowPID_D

} t_armLowerControllerConfig;

// CLASS

class ArmLowerController {

public:

    ArmBaseController(t_armLowerControllerConfig armLowerControllerConfig, t_controlMode controlMode);

    void setControlMode(t_controlMode);

    void setJointMotorSpeed(t_joint joint, float setSpeed);

    void setJointVelocity(t_joint joint, float setVelocity);

    float getJointAngle(t_joint);


private:

    t_controlMode m_controlMode;
    t_armLowerControllerConfig m_armBaseControllerConfig;

};

#endif // ARM_LOWER_CONTROLLER_H
