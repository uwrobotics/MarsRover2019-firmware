/* Controller for the arm base, shoulder and elbow
 */

#include "../inc/ArmJointController.h"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

ArmJointController::ArmJointController(t_jointConfig armJointConfig, t_jointControlMode controlMode) :
        m_controlMode(controlMode), m_armJointConfig(armJointConfig), m_motor(armJointConfig.motor.pwmPin, armJointConfig.motor.dirPin,
        armJointConfig.motor.inverted), m_encoder(armJointConfig.encoder.pwmPin), m_limSwitchMin(armJointConfig.limSwitchMinPin), m_limSwitchMax(armJointConfig.limSwitchMaxPin),
        m_velocityPIDController(armJointConfig.velocityPID.P, armJointConfig.velocityPID.I, armJointConfig.velocityPID.D, armJointConfig.velocityPID.interval),
        m_positionPIDController(armJointConfig.positionPID.P, armJointConfig.positionPID.I, armJointConfig.positionPID.D, armJointConfig.positionPID.interval) {

    if (armJointConfig.encoder.inverted) {
        m_encoderInversionMultiplier = -1;
    }
    else {
        m_encoderInversionMultiplier = 1;
    }

    initializePIDControllers();
    timer.start();

}

ArmJointController::t_jointControlMode ArmJointController::getControlMode() {
    return m_controlMode;
}

float ArmJointController::getAngleDegrees() {
    return m_encoderInversionMultiplier * 360.0f * (m_encoder.avgDutyCycle() - m_armJointConfig.encoder.zeroAngleDutyCycle);
}

float ArmJointController::getAngleVelocityDegreesPerSec() {
    return m_encoderInversionMultiplier * 360.0f * m_encoder.avgDutyCycleVelocity();
}

mbed_error_status_t ArmJointController::setControlMode(t_jointControlMode controlMode) {

    switch (controlMode) {

        case motorDutyCycle:
            m_controlMode = motorDutyCycle;
            MBED_WARN_ON_ERROR(setMotorDutyCycle(0.0f));
            break;

        case velocityPID:
            m_velocityPIDController.reset();
            m_controlMode = velocityPID;
            MBED_WARN_ON_ERROR(setVelocityDegreesPerSec(0.0f));
            break;

        case positionPID:
            m_positionPIDController.reset();
            m_controlMode = positionPID;
            MBED_WARN_ON_ERROR(setAngleDegrees(getAngleDegrees()));
            break;

        default:
            return MBED_ERROR_INVALID_ARGUMENT;
    }

    timer.reset();

    return MBED_SUCCESS;
}

mbed_error_status_t ArmJointController::setMotorDutyCycle(float dutyCycle) {
    if (m_controlMode != motorDutyCycle) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if ((m_limSwitchMin == 0 && dutyCycle < 0.0f) ||
        (m_limSwitchMax == 0 && dutyCycle > 0.0f)) {
        dutyCycle = 0.0f;
    }

    m_motor.setDutyCycle(dutyCycle);

    return MBED_SUCCESS;
}

mbed_error_status_t ArmJointController::setVelocityDegreesPerSec(float velocityDegreesPerSec) {
    if (m_controlMode != velocityPID) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if (((m_limSwitchMin == 0 || getAngleDegrees() <= m_armJointConfig.encoder.minAngleDegrees) && velocityDegreesPerSec < 0.0f) ||
        ((m_limSwitchMax == 0 || getAngleDegrees() >= m_armJointConfig.encoder.maxAngleDegrees) && velocityDegreesPerSec > 0.0f)) {
        velocityDegreesPerSec = 0.0f;
    }

    m_positionPIDController.setSetPoint(velocityDegreesPerSec);

    return MBED_SUCCESS;
}

mbed_error_status_t ArmJointController::setAngleDegrees(float angleDegrees) {
    if (m_controlMode != positionPID) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if (angleDegrees <= m_armJointConfig.encoder.minAngleDegrees) {
        angleDegrees = m_armJointConfig.encoder.minAngleDegrees;
    }
    else if (angleDegrees >= m_armJointConfig.encoder.maxAngleDegrees) {
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
            if ((m_limSwitchMin == 0 && m_motor.getDutyCycle() < 0.0f) ||
                (m_limSwitchMax == 0 && m_motor.getDutyCycle() > 0.0f)) {
                m_motor.setDutyCycle(0.0f);
            }

            break;

        case velocityPID:
            m_velocityPIDController.setInterval(interval);
            m_velocityPIDController.setProcessValue(getAngleVelocityDegreesPerSec());
            m_motor.setDutyCycle(m_velocityPIDController.compute());

            break;

        case positionPID:
            m_positionPIDController.setInterval(interval);
            m_positionPIDController.setProcessValue(getAngleDegrees());
            m_motor.setDutyCycle(m_positionPIDController.compute());

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

float ArmJointController::getMotorDutyCycle() {
    return m_motor.getDutyCycle();
}

