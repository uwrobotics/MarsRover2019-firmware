#include "ArmWristController.h"

ArmWristController::ArmWristController(t_armWristConfig armWristConfig, ArmJointController::t_jointControlMode controlMode) :
        m_controlMode(controlMode), m_leftJointController(armWristConfig.leftJointConfig, controlMode),
        m_rightJointController(armWristConfig.rightJointConfig, controlMode) {

    m_rollMotorSpeed = 0.0f;
    m_pitchMotorSpeed = 0.0f;

    m_rollVelocitiyDegreesPerSec = 0.0f;
    m_pitchVelocityDegreesPerSec = 0.0f;

    m_rollAngleDegrees = 0.0f;
    m_pitchAngleDegrees = 0.0f;
}

mbed_error_status_t ArmWristController::setControlMode(ArmJointController::t_jointControlMode controlMode) {

    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(m_leftJointController.setControlMode(controlMode));
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(m_rightJointController.setControlMode(controlMode));

    m_controlMode = controlMode;

    m_rollMotorSpeed = 0.0f;
    m_pitchMotorSpeed = 0.0f;

    m_rollVelocitiyDegreesPerSec = 0.0f;
    m_pitchVelocityDegreesPerSec = 0.0f;

    m_rollAngleDegrees = 0.0f;
    m_pitchAngleDegrees = 0.0f;

    update();

    return MBED_SUCCESS;
}

mbed_error_status_t ArmWristController::setRollDutyCycle(float dutyCycle) {
    m_rollMotorSpeed = dutyCycle;
    return setMotorSpeeds();
}

mbed_error_status_t ArmWristController::setPitchDutyCycle(float dutyCycle) {
    m_pitchMotorSpeed = dutyCycle;
    return setMotorSpeeds();
}

mbed_error_status_t ArmWristController::setRollVelocityDegreesPerSec(float velocityDegreesPerSec) {
    m_rollVelocitiyDegreesPerSec = velocityDegreesPerSec;
    return setVelocities();
}

mbed_error_status_t ArmWristController::setPitchVelocityDegreesPerSec(float velocityDegreesPerSec) {
    m_pitchVelocityDegreesPerSec = velocityDegreesPerSec;
    return setVelocities();

}

mbed_error_status_t ArmWristController::setRollAngleDegrees(float angleDegrees) {
    m_rollAngleDegrees = angleDegrees;
    return setAngles();
}

mbed_error_status_t ArmWristController::setPitchAngleDegrees(float angleDegrees) {
    m_pitchAngleDegrees = angleDegrees;
    return setAngles();
}

ArmJointController::t_jointControlMode ArmWristController::getControlMode() {
    return m_controlMode;
}

float ArmWristController::getRollAngleDegrees() {
    return (m_rightJointController.getAngleDegrees() - m_leftJointController.getAngleDegrees()) / 2.0f;
}

float ArmWristController::getPitchAngleDegrees() {
    return (m_leftJointController.getAngleDegrees() + m_rightJointController.getAngleDegrees()) / 2.0f;
}

void ArmWristController::update() {
    m_leftJointController.update();
    m_rightJointController.update();
}

mbed_error_status_t ArmWristController::setMotorSpeeds(void) {
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(m_leftJointController.setMotorDutyCycle(-m_rollMotorSpeed + m_pitchMotorSpeed));
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(m_rightJointController.setMotorDutyCycle(m_rollMotorSpeed + m_pitchMotorSpeed));

    return MBED_SUCCESS;
}

mbed_error_status_t ArmWristController::setVelocities(void) {
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(m_leftJointController.setVelocityDegreesPerSec(-m_rollVelocitiyDegreesPerSec + m_pitchVelocityDegreesPerSec));
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(m_rightJointController.setVelocityDegreesPerSec(m_rollVelocitiyDegreesPerSec + m_pitchVelocityDegreesPerSec));

    return MBED_SUCCESS;
}

mbed_error_status_t ArmWristController::setAngles(void) {
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(m_leftJointController.setAngleDegrees(-m_rollAngleDegrees + m_pitchAngleDegrees));
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(m_rightJointController.setAngleDegrees(m_rollAngleDegrees + m_pitchAngleDegrees));

    return MBED_SUCCESS;
}

