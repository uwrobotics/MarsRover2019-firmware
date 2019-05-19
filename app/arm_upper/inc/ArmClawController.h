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

        // PID config
        PID::t_pidConfig positionPID;

        float max;


    } t_clawConfig;

    typedef enum t_controlMode {
        motorDutyCycle,
        positionPID

    } t_clawControlMode;

    explicit ArmClawController(t_clawConfig armJointConfig, t_clawControlMode controlMode = positionPID);

    mbed_error_status_t setControlMode(t_clawControlMode controlMode);

    mbed_error_status_t setMotorSpeedPercent(float speedPercent);

    mbed_error_status_t setSeparationDistanceMm(float separationDistanceMm);

    mbed_error_status_t setSeparationDistanceCm(float separationDistanceCm);

    t_clawControlMode getControlMode();

    float getSeparationDistanceCm();

    void update();

protected:

    void initializePIDController(void);

    t_clawControlMode m_controlMode;
    t_clawConfig m_armClawConfig;

    Motor m_motor;
    QEI m_encoder;

    PID m_positionPIDController;

    float m_inversionMultiplier;

    Timer timer;

};

#endif // ARM_CLAW_CONTROLLER_H
