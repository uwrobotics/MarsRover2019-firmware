#ifndef ARM_LOWER_CONTROLLER_H
#define ARM_LOWER_CONTROLLER_H

/* Controller for the arm base, shoulder and elbow
 */

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"

// TYPES

typedef enum t_joint {
    baseJoint,
    elbowJoint,
    shoulderJoint

} t_joint;

typedef struct {
    PinName pwmPin;
    PinName dirPin;
    bool inverted;

} t_motorConfig;

typedef struct {
    PinName pwmPin;
    float minAngleDutyCycle;
    float zeroAngleDutyCycle;
    float maxAngleDutyCycle;
} t_encoderConfig;

typedef struct {
    float P;
    float I;
    float D;
    float
} t_pidConstants;

typedef struct {
    // Joint motor config
    t_motorConfig baseMotor;
    t_motorConfig shoulderMotor;
    t_motorConfig elbowMotor;

    // Joint encoder config
    t_encoderConfig baseEncoder;
    t_encoderConfig shoulderEncoder;
    t_encoderConfig elbowEncoder;

    // PID config
    float PIDUpdateInterval;
    t_pidConstants baseVelocityPID;
    t_pidConstants shoulderVelocityPID;
    t_pidConstants elbowVelocityPID;
    t_pidConstants basePositionPID;
    t_pidConstants shoulderPositionPID;
    t_pidConstants elbowPositionPID;

} t_armLowerControllerConfig;

typedef enum t_controlMode {
    motorSpeed,
    velocityPID,
    positionPID

} t_controlMode;

// CLASS

class ArmLowerController {

public:

    static const t_armLowerControllerConfig k_defaultArmLowerControllerConfig = {

        .baseMotor = {
            .pwmPin = MOTOR1,
            .dirPin = MOTOR1_DIR,
            .inverted = false
        },

        .shoulderMotor = {
            .pwmPin = MOTOR2,
            .directionPin = MOTOR2_DIR,
            .inverted = false
        },

        .elbowMotor = {
            .pwmPin = MOTOR3,
            .dirPin = MOTOR3_DIR,
            .inverted = false
        },

        .baseEncoder = {
            .pwmPin = ENC_A1,
            .minAngleDutyCycle = 0,
            .zeroAngleDutyCycle = 0,
            .maxAngleDutyCycle = 0
        },

        .shoulderEncoder = {
            .pwmPin = ENC_A2,
            .minAngleDutyCycle = 0,
            .zeroAngleDutyCycle = 0,
            .maxAngleDutyCycle = 0
        },

        .elbowEncoder = {
            .pwmPin = ENC_A3,
            .minAngleDutyCycle = 0,
            .zeroAngleDutyCycle = 0,
            .maxAngleDutyCycle = 0
        },

        .PIDUpdateInterval = 0.01f,

        .baseVelocityPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f
        },

        .shoulderVelocityPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f
        },

        .elbowVelocityPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f
        },

        .basePositionPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f
        },

        .shoulderPositionPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f
        },

        .elbowPositionPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f
        },

    };

    ArmBaseController(t_armLowerControllerConfig armLowerControllerConfig = k_defaultArmLowerControllerConfig, t_controlMode controlMode = t_controlMode.velocityPID);

    ArmBaseController(t_controlMode controlMode = t_controlMode.velocityPID);

    mbed_error_status_t setControlMode(t_controlMode controlMode);

    mbed_error_status_t setJointMotorSpeed(t_joint joint, float setSpeed);

    mbed_error_status_t setJointVelocity(t_joint joint, float setVelocity);

    mbed_error_status_t setJointAngle(t_joint joint, float setAngle);

    float getJointAngle(t_joint);

    void update();


private:

    t_controlMode m_controlMode;
    t_armLowerControllerConfig m_armLowerControllerConfig;

    Motor m_motor[3];
    PwmIn m_encoder[3];
    PID m_velocityPIDController[3];
    PID m_positionPIDController[3];

};

#endif // ARM_LOWER_CONTROLLER_H
