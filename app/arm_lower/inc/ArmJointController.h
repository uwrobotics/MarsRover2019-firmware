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
    float minAngleDutyCycle;
    float maxAngleDutyCycle;
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

    float PIDUpdateInterval;
    float PIDInputVelocityMinDegPerSec, PIDInputVelocityMaxDegPerSec;
    float PIDOutputMotorMinDutyCycle, PIDOutputMotorMaxDutyCycle;

} t_armJointConfig;

typedef enum t_controlMode {
    motorDutyCycle,
    velocityPID,
    positionPID

} t_controlMode;

// CLASS

class ArmJointController {

public:

    ArmJointController(t_armJointConfig armJointConfig, t_controlMode controlMode = velocityPID);

    mbed_error_status_t setControlMode(t_controlMode controlMode);

    mbed_error_status_t setMotorSpeedPercent(float speedPercent);

    mbed_error_status_t setVelocityDegreesPerSec(float velocityDegreesPerSec);

    mbed_error_status_t setAngleDegrees(float angleDegrees);

    t_controlMode getControlMode();

    float getAngle();

    void update();

protected:

    void initializePIDControllers(void);

    t_controlMode m_controlMode;
    t_armJointConfig m_armJointConfig;

    Motor m_motor;
    PwmIn m_encoder;

    float m_prevEncoderPWMDuty;

    PID m_velocityPIDController;
    PID m_positionPIDController;

    Timer timer;

};

#endif // ARM_JOINT_CONTROLLER_H
