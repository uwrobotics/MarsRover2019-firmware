#ifndef ARM_JOINT_CONTROLLER_H
#define ARM_JOINT_CONTROLLER_H

/* Controller for the arm base, shoulder and elbow
 */

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"
#include "ArmJointController.h"

// TYPES

typedef struct {
    // Joint config
    t_armJointConfig leftJointConfig, rightJointConfig;

    float initPIDUpdateInterval;
    float PIDInputVelocityMinDegPerSec, PIDInputVelocityMaxDegPerSec;
    float PIDOutputMotorMinDutyCycle, PIDOutputMotorMaxDutyCycle;

} t_armWristConfig;

typedef enum t_controlMode {
    motorDutyCycle,
    velocityPID,
    positionPID

} t_controlMode;

// CLASS

class ArmWristController {

public:

    ArmWristController(t_armWristConfig armWristConfig, t_controlMode controlMode = velocityPID);

    mbed_error_status_t setControlMode(t_controlMode controlMode);

    mbed_error_status_t setRollSpeedPercent(float speedPercent);

    mbed_error_status_t setPitchSpeedPercent(float speedPercent);

    mbed_error_status_t setRollVelocityDegreesPerSec(float velocityDegreesPerSec);

    mbed_error_status_t setPitchVelocityDegreesPerSec(float velocityDegreesPerSec);

    mbed_error_status_t setRollAngleDegrees(float angleDegrees);

    mbed_error_status_t setPitchAngleDegrees(float angleDegrees);

    t_controlMode getControlMode();

    float getRollAngleDegrees();

    float getPitchAngleDegrees();

    float getRollAngleVelocityDegreesPerSec();

    float getPitchAngleVelocityDegreesPerSec();

    void update();

protected:

    void initializePIDControllers(void);

    t_controlMode m_controlMode;
    t_armJointConfig m_armJointConfig;

    Motor m_motor;
    PwmIn m_encoder;

    PID m_velocityPIDController;
    PID m_positionPIDController;

    float m_inversionMultiplier;

    Timer timer;

};

#endif // ARM_JOINT_CONTROLLER_H
