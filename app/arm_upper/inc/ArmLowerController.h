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
    float P;
    float I;
    float D;
} t_pidConstants;

typedef struct {
    float minAngleDutyCycle;
    float zeroAngleDutyCycle;
    float maxAngleDutyCycle;
} t_absoluteEncoderConfig;

typedef struct {
    // Motor config
    PinName baseMotor;
    PinName shoulderMotor;
    PinName elbowMotor;

    // Motor direction config
    PinName baseMotorDir;
    PinName shoulderMotorDir;
    PinName elbowMotorDir;

    // Joint encoder config
    PinName baseEncoder;
    PinName shoulderEncoder;
    PinName elbowEncoder;

    // Encoder duty cycle angle config (minimum, zero, and maximum angle)
    t_absoluteEncoderConfig baseEncoderConfig;
    t_absoluteEncoderConfig shoulderEncoderConfig;
    t_absoluteEncoderConfig elbowEncoderConfig;

    // PID config
    float PIDUpdateInterval;
    t_pidConstants baseVelocityPID;
    t_pidConstants shoulderVelocityPID;
    t_pidConstants elbowVelocityPID;
    t_pidConstants basePositionPID;
    t_pidConstants shoulderPositionPID;
    t_pidConstants elbowPositionPID;

} t_armJointConfig;

// CLASS

class ArmLowerController {

public:

    ArmBaseController(t_armLowerControllerConfig armLowerControllerConfig, t_controlMode controlMode);

    void setControlMode(t_controlMode);

    void setJointMotorSpeed(t_joint joint, float setSpeed);

    void setJointVelocity(t_joint joint, float setVelocity);

    void setJointAngle(t_joint joint, float setAngle);

    float getJointAngle(t_joint);

    void update();

private:

    t_controlMode m_controlMode;
    t_armJointConfig m_armLowerControllerConfig;

};

#endif // ARM_LOWER_CONTROLLER_H
