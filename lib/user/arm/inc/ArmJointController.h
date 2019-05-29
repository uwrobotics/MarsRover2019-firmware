#ifndef ARM_JOINT_CONTROLLER_H
#define ARM_JOINT_CONTROLLER_H

/* Controller for the arm base, shoulder and elbow
 */

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

// CLASS

class ArmJointController {

public:

    // TYPES

    typedef struct {
        // Joint motor config
        Motor::t_motorConfig motor;

        // Joint encoder config
        PwmIn::t_absoluteEncoderConfig encoder;

        // PID config
        PID::t_pidConfig velocityPID, positionPID;

        float minInputVelocityDegPerSec, maxInputVelocityDegPerSec;
        float minOutputMotorDutyCycle, maxOutputMotorDutyCycle;

    } t_jointConfig;

    typedef enum t_controlMode {
        motorDutyCycle,
        velocityPID,
        positionPID

    } t_jointControlMode;

    explicit ArmJointController(t_jointConfig armJointConfig, t_jointControlMode controlMode = motorDutyCycle);

    mbed_error_status_t setControlMode(t_jointControlMode controlMode);

    mbed_error_status_t setMotorSpeedPercent(float speedPercent);

    mbed_error_status_t setVelocityDegreesPerSec(float velocityDegreesPerSec);

    mbed_error_status_t setAngleDegrees(float angleDegrees);

    t_jointControlMode getControlMode();

    float getAngleDegrees();

    float getAngleVelocityDegreesPerSec();

    void update();

private:

    void initializePIDControllers(void);

    t_jointControlMode m_controlMode;
    t_jointConfig m_armJointConfig;

    Motor m_motor;
    PwmIn m_encoder;

    PID m_velocityPIDController;
    PID m_positionPIDController;

    float m_encoderInversionMultiplier;

    Timer timer;

};

#endif // ARM_JOINT_CONTROLLER_H
