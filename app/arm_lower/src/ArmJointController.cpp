/* Controller for the arm base, shoulder and elbow
 */

#include "ArmJointController.h"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

ArmJointController::ArmJointController(t_armJointConfig armJointConfig, t_controlMode controlMode) :
    m_controlMode(controlMode), m_armJointConfig(armJointConfig),
    m_motor(armJointConfig.motor.pwmPin, armJointConfig.motor.dirPin, armJointConfig.motor.inverted), m_encoder(armJointConfig.encoder.pwmPin),
    m_velocityPIDController(armJointConfig.velocityPID.P, armJointConfig.velocityPID.I, armJointConfig.velocityPID.D, armJointConfig.PIDUpdateInterval),
    m_positionPIDController(armJointConfig.positionPID.P, armJointConfig.positionPID.I, armJointConfig.positionPID.D, armJointConfig.PIDUpdateInterval) {

    initializePIDControllers();
    timer.start();

}

mbed_error_status_t ArmJointController::setControlMode(t_controlMode controlMode) {
    m_controlMode = controlMode;
    m_motor.speed(0.0f);

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

    m_motor.speed(speedPercent);

    return MBED_SUCCESS;
}

mbed_error_status_t ArmJointController::setVelocityDegreesPerSec(float velocityDegreesPerSec) {
    if (m_controlMode != velocityPID) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    m_positionPIDController.setSetPoint(velocityDegreesPerSec);

    return MBED_SUCCESS;
}

mbed_error_status_t ArmJointController::setAngleDegrees(float angleDegrees) {
    if (m_controlMode != positionPID) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    m_positionPIDController.setSetPoint(angleDegrees);

    return MBED_SUCCESS;
}

t_controlMode ArmJointController::getControlMode() {
    return m_controlMode;
}

float ArmJointController::getAngle() {
    return (m_encoder.avgDutyCycle() - m_armJointConfig.encoder.zeroAngleDutyCycle) * 360.0f;
}

void ArmJointController::update() {
    float encoderPWMDuty = m_encoder.avgDutyCycle();;
    float angularVelocity = 0;
    float interval = timer.read();

    timer.reset();
    m_prevEncoderPWMDuty = encoderPWMDuty;

    switch (m_controlMode) {
        case motorDutyCycle:
            break;
        case velocityPID:

            angularVelocity = 360.0f * (encoderPWMDuty - m_prevEncoderPWMDuty) / interval;

            m_velocityPIDController.setInterval(interval);
            m_velocityPIDController.setProcessValue(angularVelocity);

            m_motor.speed(m_velocityPIDController.compute());

            break;
        case positionPID:

            m_positionPIDController.setInterval(interval);
            m_positionPIDController.setProcessValue(encoderPWMDuty);

            m_motor.speed(m_velocityPIDController.compute());

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
    m_positionPIDController.setInputLimits(m_armJointConfig.encoder.minAngleDutyCycle * 360.0f, m_armJointConfig.encoder.maxAngleDutyCycle * 360.0f);
    m_velocityPIDController.setOutputLimits(m_armJointConfig.PIDOutputMotorMinDutyCycle, m_armJointConfig.PIDOutputMotorMaxDutyCycle);
    m_velocityPIDController.setBias(m_armJointConfig.positionPID.bias);
    m_velocityPIDController.setMode(PID_AUTO_MODE);
}

