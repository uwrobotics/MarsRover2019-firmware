#ifndef ARM_CLAW_CONTROLLER_H
#define ARM_CLAW_CONTROLLER_H

/* Controller for the arm claw
 */

#include "mbed.h"
#include "Motor.h"
#include "QEI.h"
#include "PID.h"
#include "PinNames.h"

// CLASS

class ArmClawController {

public:

    // TYPES

    typedef struct {
        // Claw motor config
        Motor::t_motorConfig motor;

        // Claw encoder config
        QEI::t_relativeEncoderConfig encoder;

        // Limit switch pin
        PinName limitSwitchPin;
        float calibrationDutyCycle, calibrationTimeoutSeconds;

        // PID config
        PID::t_pidConfig positionPID;

        float minInputSeparationDistanceCm, maxInputSeparationDistanceCm;
        float minOutputMotorDutyCycle, maxOutputMotorDutyCycle;

    } t_clawConfig;

    typedef enum t_controlMode {
        motorDutyCycle,
        positionPID

    } t_clawControlMode;

    explicit ArmClawController(t_clawConfig armClawConfig, t_clawControlMode controlMode = motorDutyCycle);

    mbed_error_status_t setControlMode(t_clawControlMode controlMode);

    mbed_error_status_t setMotorSpeedPercent(float speedPercent);

    mbed_error_status_t setSeparationDistanceMm(float separationDistanceMm);

    mbed_error_status_t setSeparationDistanceCm(float separationDistanceCm);

    t_clawControlMode getControlMode();

    float getSeparationDistanceMm();

    float getSeparationDistanceCm();

    mbed_error_status_t runEndpointCalibration();

    void update();

private:

    void initializePIDController(void);

    float encoderPulsesToMm(int encoderPulses);

    t_clawControlMode m_controlMode;
    t_clawConfig m_armClawConfig;

    Motor m_motor;
    QEI m_encoder;
    DigitalIn m_limitSwitch;

    PID m_positionPIDController;

    float m_inversionMultiplier;
    bool m_encoderEndpointCalibrated;

    Timer timer;

};

#endif // ARM_CLAW_CONTROLLER_H
