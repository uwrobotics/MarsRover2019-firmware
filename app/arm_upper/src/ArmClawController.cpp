
#include <ArmClawController.h>

#include "ArmClawController.h"

const float k_PulsesToCmA = 121;
const float k_PulsesToCmB = -9.35E-05;
const float k_PulsesToCmC = -1.11E-07;
const float k_PulsesToCmD = 1.08E-12;

ArmClawController::ArmClawController(ArmClawController::t_clawConfig armClawConfig, ArmClawController::t_clawControlMode controlMode) :
         m_controlMode(controlMode), m_armClawConfig(armClawConfig), m_motor(armClawConfig.motor), m_encoder(armClawConfig.encoder), m_limitSwitch(armClawConfig.limitSwitchPin),
         m_positionPIDController(armClawConfig.positionPID.P, armClawConfig.positionPID.I, armClawConfig.positionPID.D, armClawConfig.positionPID.interval) {

    m_encoderEndpointCalibrated = false;
    initializePIDController();
    timer.start();

}

mbed_error_status_t ArmClawController::setControlMode(ArmClawController::t_clawControlMode controlMode) {

    switch (m_controlMode) {

        case motorDutyCycle:
            m_controlMode = motorDutyCycle;
            setMotorDutyCycle(0.0f);
            break;

        case positionPID:
            m_positionPIDController.reset();
            m_controlMode = positionPID;
            runEndpointCalibration();
            break;

        default:
            return MBED_ERROR_CODE_INVALID_ARGUMENT;
    }

    m_controlMode = controlMode;
    m_motor.setDutyCycle(0.0f);

    timer.reset();

    return MBED_SUCCESS;
}

mbed_error_status_t ArmClawController::setMotorDutyCycle(float dutyCycle) {
    if (m_controlMode != motorDutyCycle || !m_encoderEndpointCalibrated) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if ((getSeparationDistanceCm() < m_armClawConfig.minInputSeparationDistanceCm && dutyCycle < 0.0f) ||
        (getSeparationDistanceCm() > m_armClawConfig.maxInputSeparationDistanceCm && dutyCycle > 0.0f)) {
        dutyCycle = 0.0f;
    }

    m_motor.setDutyCycle(dutyCycle);

    return MBED_SUCCESS;
}

mbed_error_status_t ArmClawController::setSeparationDistanceMm(float separationDistanceMm) {
    if (m_controlMode != positionPID || !m_encoderEndpointCalibrated) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if (getSeparationDistanceCm() < m_armClawConfig.minInputSeparationDistanceCm) {
        separationDistanceMm = m_armClawConfig.minInputSeparationDistanceCm;
    }
    else if (getSeparationDistanceCm() > m_armClawConfig.maxInputSeparationDistanceCm) {
        separationDistanceMm = m_armClawConfig.maxInputSeparationDistanceCm;
    }

    m_positionPIDController.setSetPoint(separationDistanceMm);

    return MBED_SUCCESS;
}

mbed_error_status_t ArmClawController::setSeparationDistanceCm(float separationDistanceCm) {
    return setSeparationDistanceMm(separationDistanceCm * 10.0);
}

ArmClawController::t_clawControlMode ArmClawController::getControlMode() {
    return m_controlMode;
}

float ArmClawController::encoderPulsesToMm(int encoderPulses) {
    return k_PulsesToCmA + k_PulsesToCmB * encoderPulses + k_PulsesToCmC * pow(encoderPulses, 2) + k_PulsesToCmD * pow(encoderPulses, 3);
}

float ArmClawController::getSeparationDistanceMm() {
    return encoderPulsesToMm(m_encoder.getPulses());
}

float ArmClawController::getSeparationDistanceCm() {
    return getSeparationDistanceMm();
}

void ArmClawController::update() {
    float interval = timer.read();
    timer.reset();

    switch (m_controlMode) {
        case motorDutyCycle:
            if ((getSeparationDistanceCm() < m_armClawConfig.minInputSeparationDistanceCm &&
                    m_motor.getDutyCycle() < 0.0f) ||
                (getSeparationDistanceCm() > m_armClawConfig.maxInputSeparationDistanceCm &&
                        m_motor.getDutyCycle() > 0.0f)) {
                m_motor.setDutyCycle(0.0f);
            }

            break;

        case positionPID:
            m_positionPIDController.setInterval(interval);
            m_positionPIDController.setProcessValue(getSeparationDistanceMm());
            m_motor.setDutyCycle(m_positionPIDController.compute());

            break;
    }
}

void ArmClawController::initializePIDController() {
    // Configure position PID
    m_positionPIDController.setInputLimits(m_armClawConfig.minInputSeparationDistanceCm, m_armClawConfig.maxInputSeparationDistanceCm);
    m_positionPIDController.setOutputLimits(m_armClawConfig.minOutputMotorDutyCycle, m_armClawConfig.maxOutputMotorDutyCycle);
    m_positionPIDController.setBias(m_armClawConfig.positionPID.bias);
    m_positionPIDController.setMode(PID_AUTO_MODE);
}

mbed_error_status_t ArmClawController::runEndpointCalibration() {

    t_controlMode prevControlMode = getControlMode();

    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(setControlMode(motorDutyCycle));
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR(setMotorDutyCycle(m_armClawConfig.calibrationDutyCycle));

    timer.reset();

    while (m_limitSwitch.read() == 0) {
        if (timer.read() > m_armClawConfig.calibrationTimeoutSeconds) {
            return MBED_ERROR_TIME_OUT;
        }
    }

    m_encoder.reset();

    setControlMode(prevControlMode);

    return MBED_SUCCESS;
}
