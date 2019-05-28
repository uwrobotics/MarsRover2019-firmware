/* Controller for the arm base, shoulder and elbow
 */

#include "../inc/ArmJointController.h"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

ArmJointController::ArmJointController(t_jointConfig armJointConfig, t_jointControlMode controlMode) :
        m_controlMode(controlMode), m_armJointConfig(armJointConfig),
        m_motor(armJointConfig.motor.pwmPin, armJointConfig.motor.dirPin, armJointConfig.motor.inverted), m_encoder(armJointConfig.encoder.pwmPin),
        m_velocityPIDController(armJointConfig.velocityPID.P, armJointConfig.velocityPID.I, armJointConfig.velocityPID.D, armJointConfig.velocityPID.interval),
        m_positionPIDController(armJointConfig.positionPID.P, armJointConfig.positionPID.I, armJointConfig.positionPID.D, armJointConfig.positionPID.interval) {

    if (armJointConfig.encoder.inverted) {
        m_inversionMultiplier = -1;
    }
    else {
        m_inversionMultiplier = 1;
    }

    initializePIDControllers();
    timer.start();

}

ArmJointController::t_jointControlMode ArmJointController::getControlMode() {
    return m_controlMode;
}

float ArmJointController::getAngleDegrees() {
    return m_inversionMultiplier * 360.0f * (m_encoder.avgDutyCycle() - m_armJointConfig.encoder.zeroAngleDutyCycle);
}

float ArmJointController::getAngleVelocityDegreesPerSec() {
    return m_inversionMultiplier * 360.0f * m_encoder.avgDutyCycleVelocity();
}

mbed_error_status_t ArmJointController::setControlMode(t_jointControlMode controlMode) {

    switch (m_controlMode) {

        case motorDutyCycle:
            m_controlMode = motorDutyCycle;
            setMotorSpeedPercent(0.0f);
            break;

        case velocityPID:
            m_velocityPIDController.reset();
            m_controlMode = velocityPID;
            setVelocityDegreesPerSec(0.0f);
            break;

        case positionPID:
            m_positionPIDController.reset();
            m_controlMode = positionPID;
            setAngleDegrees(getAngleDegrees());
            break;

        default:
            return MBED_ERROR_INVALID_ARGUMENT;
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
            m_motor.setSpeed(m_positionPIDController.compute());

            break;
    }
}

void ArmJointController::initializePIDControllers(void) {

    // Configure velocity PID
    m_velocityPIDController.setInputLimits(m_armJointConfig.minInputVelocityDegPerSec, m_armJointConfig.maxInputVelocityDegPerSec);
    m_velocityPIDController.setOutputLimits(m_armJointConfig.minOutputMotorDutyCycle, m_armJointConfig.maxOutputMotorDutyCycle);
    m_velocityPIDController.setBias(m_armJointConfig.velocityPID.bias);
    m_velocityPIDController.setMode(PID_AUTO_MODE);

    // Configure position PID
    m_positionPIDController.setInputLimits(m_armJointConfig.encoder.minAngleDegrees, m_armJointConfig.encoder.maxAngleDegrees);
    m_positionPIDController.setOutputLimits(m_armJointConfig.minOutputMotorDutyCycle, m_armJointConfig.maxOutputMotorDutyCycle);
    m_positionPIDController.setBias(m_armJointConfig.positionPID.bias);
    m_positionPIDController.setMode(PID_AUTO_MODE);
}

