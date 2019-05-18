#include "ArmWristController.h"

#define MBED_ASSERT_SUCCESS_RETURN_ERROR(functionCall) {     \
    mbed_error_status_t result = functionCall;  \
    if (result != MBED_SUCCESS) {               \
        return result;                          \
    }                                           \
}

ArmWristController::ArmWristController(t_armWristConfig armWristConfig, t_jointControlMode controlMode) :
    m_controlMode(controlMode),
    m_leftJointController(armWristConfig.leftJointConfig, controlMode),
    m_rightJointController(armWristConfig.rightJointConfig, controlMode) {}

mbed_error_status_t ArmWristController::setControlMode(t_jointControlMode controlMode) {
    m_controlMode = controlMode;

    m_rollMotorSpeed = 0.0f;
    m_pitchMotorSpeed = 0.0f;

    m_rollVelocitiyDegreesPerSec = 0.0f;
    m_pitchVelocityDegreesPerSec = 0.0f;

    m_rollAngleDegrees = 0.0f;
    m_pitchAngleDegrees = 0.0f;

    update();

    MBED_ASSERT_SUCCESS_RETURN_ERROR(m_leftJointController.setControlMode(controlMode));
    MBED_ASSERT_SUCCESS_RETURN_ERROR(m_rightJointController.setControlMode(controlMode));

    return MBED_SUCCESS;
}

mbed_error_status_t ArmWristController::setRollSpeedPercent(float speedPercent) {
    m_rollMotorSpeed = speedPercent;
    return setMotorSpeeds();
}

mbed_error_status_t ArmWristController::setPitchSpeedPercent(float speedPercent) {
    m_pitchMotorSpeed = speedPercent;
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

t_jointControlMode ArmWristController::getControlMode() {
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
    MBED_ASSERT_SUCCESS_RETURN_ERROR(m_leftJointController.setMotorSpeedPercent(-m_rollMotorSpeed + m_pitchMotorSpeed));
    MBED_ASSERT_SUCCESS_RETURN_ERROR(m_rightJointController.setMotorSpeedPercent(m_rollMotorSpeed + m_pitchMotorSpeed));

    return MBED_SUCCESS;
}

mbed_error_status_t ArmWristController::setVelocities(void) {
    MBED_ASSERT_SUCCESS_RETURN_ERROR(m_leftJointController.setVelocityDegreesPerSec(-m_rollVelocitiyDegreesPerSec + m_pitchVelocityDegreesPerSec));
    MBED_ASSERT_SUCCESS_RETURN_ERROR(m_rightJointController.setVelocityDegreesPerSec(m_rollVelocitiyDegreesPerSec + m_pitchVelocityDegreesPerSec));

    return MBED_SUCCESS;
}

mbed_error_status_t ArmWristController::setAngles(void) {
    MBED_ASSERT_SUCCESS_RETURN_ERROR(m_leftJointController.setAngleDegrees(-m_rollAngleDegrees + m_pitchAngleDegrees));
    MBED_ASSERT_SUCCESS_RETURN_ERROR(m_rightJointController.setAngleDegrees(m_rollAngleDegrees + m_pitchAngleDegrees));

    return MBED_SUCCESS;
}

