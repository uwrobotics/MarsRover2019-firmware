/* Controller for the arm base, shoulder and elbow
 */

#include "../inc/ArmJointController.h"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

ArmJointController::ArmJointController(t_armJointConfig armJointConfig, t_controlMode controlMode) :
    m_controlMode(controlMode), m_armJointConfig(armJointConfig),
    m_motor(armJointConfig.motor.pwmPin, armJointConfig.motor.dirPin, armJointConfig.motor.inverted), m_encoder(armJointConfig.encoder.pwmPin),
    m_velocityPIDController(armJointConfig.velocityPID.P, armJointConfig.velocityPID.I, armJointConfig.velocityPID.D, armJointConfig.initPIDUpdateInterval),
    m_positionPIDController(armJointConfig.positionPID.P, armJointConfig.positionPID.I, armJointConfig.positionPID.D, armJointConfig.initPIDUpdateInterval) {

    if (armJointConfig.encoder.inverted) {
        m_inversionMultiplier = -1;
    }
    else {
        m_inversionMultiplier = 1;
    }

    initializePIDControllers();
    timer.start();

}

t_controlMode ArmJointController::getControlMode() {
    return m_controlMode;
}

float ArmJointController::getAngleDegrees() {
    return m_inversionMultiplier * 360.0f * (m_encoder.avgDutyCycle() - m_armJointConfig.encoder.zeroAngleDutyCycle);
}

float ArmJointController::getAngleVelocityDegreesPerSec() {
    return m_inversionMultiplier * 360.0f * m_encoder.avgDutyCycleVelocity();
}

mbed_error_status_t ArmJointController::setControlMode(t_controlMode controlMode) {
    m_controlMode = controlMode;
    m_motor.setSpeed(0.0f);

    switch (m_controlMode) {
        case motorDutyCycle:
            break;
        case velocityPID:
            m_velocityPIDController.reset();

            break;
        case positionPID:
            m_positionPIDController.reset();

            break;
    }

    timer.reset();

    return MBED_SUCCESS;
}

mbed_error_status_t ArmJointController::setMotorSpeedPercent(float speedPercent) {
    if (m_controlMode != motorDutyCycle) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if ((getAngleDegrees() < m_armJointConfig.encoder.minAngleDegrees && speedPercent < 0.0f) ||
        (getAngleDegrees() > m_armJointConfig.encoder.maxAngleDegrees && speedPercent > 0.0f)) {
        speedPercent = 0.0f;
    }

    m_motor.setSpeed(speedPercent);

    return MBED_SUCCESS;
}

mbed_error_status_t ArmJointController::setVelocityDegreesPerSec(float velocityDegreesPerSec) {
    if (m_controlMode != velocityPID) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if ((getAngleDegrees() < m_armJointConfig.encoder.minAngleDegrees && velocityDegreesPerSec < 0.0f) ||
        (getAngleDegrees() > m_armJointConfig.encoder.maxAngleDegrees && velocityDegreesPerSec > 0.0f)) {
        velocityDegreesPerSec = 0.0f;
    }

    m_positionPIDController.setSetPoint(velocityDegreesPerSec);

    return MBED_SUCCESS;
}

mbed_error_status_t ArmJointController::setAngleDegrees(float angleDegrees) {
    if (m_controlMode != positionPID) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if (angleDegrees < m_armJointConfig.encoder.minAngleDegrees) {
        angleDegrees = m_armJointConfig.encoder.minAngleDegrees;
    }
    else if (angleDegrees > m_armJointConfig.encoder.maxAngleDegrees) {
        angleDegrees = m_armJointConfig.encoder.maxAngleDegrees;
    }

    m_positionPIDController.setSetPoint(angleDegrees);

    return MBED_SUCCESS;
}

void ArmJointController::update() {
    float interval = timer.read();
    timer.reset();

    switch (m_controlMode) {
        case motorDutyCycle:
            if ((getAngleDegrees() < m_armJointConfig.encoder.minAngleDegrees && m_motor.getSpeed() < 0.0f) ||
                (getAngleDegrees() > m_armJointConfig.encoder.maxAngleDegrees && m_motor.getSpeed() > 0.0f)) {
                m_motor.setSpeed(0.0f);
            }

            break;

        case velocityPID:
            m_velocityPIDController.setInterval(interval);
            m_velocityPIDController.setProcessValue(getAngleVelocityDegreesPerSec());
            m_motor.setSpeed(m_velocityPIDController.compute());

            break;

        case positionPID:
            m_positionPIDController.setInterval(interval);
            m_positionPIDController.setProcessValue(getAngleDegrees());
            m_motor.setSpeed(m_velocityPIDController.compute());

            break;
    }
}

void ArmJointController::initializePIDControllers(void) {

    // Configure velocity PID
    m_velocityPIDController.setInputLimits(m_armJointConfig.PIDInputVelocityMinDegPerSec, m_armJointConfig.PIDInputVelocityMaxDegPerSec);
    m_velocityPIDController.setOutputLimits(m_armJointConfig.PIDOutputMotorMinDutyCycle, m_armJointConfig.PIDOutputMotorMaxDutyCycle);
    m_velocityPIDController.setBias(m_armJointConfig.velocityPID.bias);
    m_velocityPIDController.setMode(PID_AUTO_MODE);

    // Configure position PID
    m_positionPIDController.setInputLimits(m_armJointConfig.encoder.minAngleDegrees * 360.0f, m_armJointConfig.encoder.maxAngleDegrees * 360.0f);
    m_velocityPIDController.setOutputLimits(m_armJointConfig.PIDOutputMotorMinDutyCycle, m_armJointConfig.PIDOutputMotorMaxDutyCycle);
    m_velocityPIDController.setBias(m_armJointConfig.positionPID.bias);
    m_velocityPIDController.setMode(PID_AUTO_MODE);
}

