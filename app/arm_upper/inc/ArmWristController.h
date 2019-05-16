#ifndef ARM_WRIST_CONTROLLER_H
#define ARM_WRIST_CONTROLLER_H

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

    float leftToRightMotorBias;


} t_armWristConfig;

// CLASS

class ArmWristController {

public:

    explicit ArmWristController(t_armWristConfig armWristConfig, t_jointControlMode controlMode = velocityPID);

    mbed_error_status_t setControlMode(t_jointControlMode controlMode);

    mbed_error_status_t setRollSpeedPercent(float speedPercent);

    mbed_error_status_t setPitchSpeedPercent(float speedPercent);

    mbed_error_status_t setRollVelocityDegreesPerSec(float velocityDegreesPerSec);

    mbed_error_status_t setPitchVelocityDegreesPerSec(float velocityDegreesPerSec);

    mbed_error_status_t setRollAngleDegrees(float angleDegrees);

    mbed_error_status_t setPitchAngleDegrees(float angleDegrees);
return 0;
    t_jointControlMode getControlMode();

    float getRollAngleDegrees();

    float getPitchAngleDegrees();

    float getRollAngleVelocityDegreesPerSec();

    float getPitchAngleVelocityDegreesPerSec();

    void update();

protected:

    mbed_error_status_t setMotorSpeeds(void);
    mbed_error_status_t setVelocities(void);
    mbed_error_status_t setAngles(void);

    t_jointControlMode m_controlMode;

    ArmJointController m_leftJointController, m_rightJointController;

    float m_rollMotorSpeed, m_pitchMotorSpeed;
    float m_rollVelocitiyDegreesPerSec, m_pitchVelocityDegreesPerSec;
    float m_rollAngleDegrees, m_pitchAngleDegrees;

    Timer timer;

};

#endif // ARM_WRIST_CONTROLLER_H
