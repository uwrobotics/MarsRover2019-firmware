#ifndef ARM_JOINT_CONTROLLER_H
#define ARM_JOINT_CONTROLLER_H

/* Controller for the arm base, shoulder and elbow
 */

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

// TYPES

typedef struct {
    PinName pwmPin;
    PinName dirPin;
    bool inverted;

} t_motorConfig;

typedef struct {
    PinName pwmPin;
    float zeroAngleDutyCycle;
    float minAngleDegrees;
    float maxAngleDegrees;
    bool inverted;
} t_absoluteEncoderConfig;

typedef struct {
    float P, I, D, bias;
} t_pidConstants;

typedef struct {
    // Joint motor config
    t_motorConfig motor;

    // Joint encoder config
    t_absoluteEncoderConfig encoder;

    // PID config
    t_pidConstants velocityPID, positionPID;

    float initPIDUpdateInterval;
    float PIDInputVelocityMinDegPerSec, PIDInputVelocityMaxDegPerSec;
    float PIDOutputMotorMinDutyCycle, PIDOutputMotorMaxDutyCycle;

} t_armJointConfig;

typedef enum t_controlMode {
    motorDutyCycle,
    velocityPID,
    positionPID

} t_jointControlMode;

// CLASS

class ArmJointController {

public:

    explicit ArmJointController(t_armJointConfig armJointConfig, t_jointControlMode controlMode = velocityPID);

    mbed_error_status_t setControlMode(t_jointControlMode controlMode);

    mbed_error_status_t setMotorSpeedPercent(float speedPercent);

    mbed_error_status_t setVelocityDegreesPerSec(float velocityDegreesPerSec);

    mbed_error_status_t setAngleDegrees(float angleDegrees);

    t_jointControlMode getControlMode();

    float getAngleDegrees();

    float getAngleVelocityDegreesPerSec();

    void update();

protected:

    void initializePIDControllers(void);

    t_jointControlMode m_controlMode;
    t_armJointConfig m_armJointConfig;

    Motor m_motor;
    PwmIn m_encoder;

    PID m_velocityPIDController;
    PID m_positionPIDController;

    float m_inversionMultiplier;

    Timer timer;

};

#endif // ARM_JOINT_CONTROLLER_H
