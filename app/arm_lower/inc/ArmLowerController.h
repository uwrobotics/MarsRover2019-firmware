#ifndef ARM_LOWER_CONTROLLER_H
#define ARM_LOWER_CONTROLLER_H

/* Controller for the arm base, shoulder and elbow
 */

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"
#include "ArmJointController.h"

// TYPES

typedef enum t_joint {
    baseJoint,
    elbowJoint,
    shoulderJoint

} t_joint;

typedef struct {
    PinName pwmPin;
    float minAngleDutyCycle;
    float zeroAngleDutyCycle;
    float maxAngleDutyCycle;
} t_absoluteEncoderConfig;

typedef struct {
    float P, I, D;
} t_pidConstants;

typedef struct {
    // Joint motor config
    t_motorConfig baseMotor, shoulderMotor, elbowMotor;

    // Joint encoder config
    t_absoluteEncoderConfig baseEncoder, shoulderEncoder, elbowEncoder;

    // PID config
    t_pidConstants baseVelocityPID, shoulderVelocityPID, elbowVelocityPID;
    t_pidConstants basePositionPID, shoulderPositionPID, elbowPositionPID;

    float PIDUpdateInterval;
    float PIDInputVelocityMinDegPerSec, PIDInputVelocityMaxDegPerSec;
    float PIDOutputMotorMinDutyCycle, PIDOutputMotorMaxDutyCycle;

} t_armJointConfig;

typedef enum t_controlMode {
    motorSpeed,
    velocityPID,
    positionPID

} t_controlMode;

// CLASS

class ArmLowerController {

public:

    static constexpr t_armJointConfig k_defaultArmLowerControllerConfig = {

        .baseMotor = {
            .pwmPin   = MOTOR1,
            .dirPin   = MOTOR1_DIR,
            .inverted = false
        },

        .shoulderMotor = {
            .pwmPin   = MOTOR2,
            .dirPin   = MOTOR2_DIR,
            .inverted = false
        },

        .elbowMotor = {
            .pwmPin   = MOTOR3,
            .dirPin   = MOTOR3_DIR,
            .inverted = false
        },

        .baseEncoder = {
            .pwmPin             = ENC_A1,
            .minAngleDutyCycle  = 0,
            .zeroAngleDutyCycle = 0,
            .maxAngleDutyCycle  = 0
        },

        .shoulderEncoder = {
            .pwmPin             = ENC_A2,
            .minAngleDutyCycle  = 0,
            .zeroAngleDutyCycle = 0,
            .maxAngleDutyCycle  = 0
        },

        .elbowEncoder = {
            .pwmPin             = ENC_A3,
            .minAngleDutyCycle  = 0,
            .zeroAngleDutyCycle = 0,
            .maxAngleDutyCycle  = 0
        },

        .baseVelocityPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f,
        },

        .shoulderVelocityPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f,
        },

        .elbowVelocityPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f,
        },

        .basePositionPID = {
            .P = 0.0f,
            .I = 0.0f,
            .D = 0.0f,
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

        .PIDUpdateInterval = 0.01f,

        .PIDInputVelocityMinDegPerSec = -5.0f,
        .PIDInputVelocityMaxDegPerSec = 5.0f,

        .PIDOutputMotorMinDutyCycle = -1.0f,
        .PIDOutputMotorMaxDutyCycle = 1.0f

    };

    ArmLowerController(t_controlMode controlMode = velocityPID, t_armJointConfig armLowerControllerConfig = k_defaultArmLowerControllerConfig);

    mbed_error_status_t setControlMode(t_controlMode controlMode);

    mbed_error_status_t setJointMotorSpeed(t_joint joint, float setSpeed);

    mbed_error_status_t setJointVelocity(t_joint joint, float setVelocity);

    mbed_error_status_t setJointAngle(t_joint joint, float setAngle);

    float getJointAngle(t_joint);

    void update();

protected:

    t_controlMode m_controlMode;
    t_armJointConfig m_armLowerControllerConfig;

    std::vector<*Motor> m_motor;
    std::vector<PwmIn> m_encoder;

    std::vector<PID> m_velocityPIDController;
    std::vector<PID> m_positionPIDController;

};

#endif // ARM_LOWER_CONTROLLER_H
