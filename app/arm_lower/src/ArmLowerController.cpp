#include "ArmLowerController.h"

ArmLowerController::ArmLowerController(t_controlMode controlMode, t_armJointConfig armLowerControllerConfig) :
        m_controlMode(controlMode), m_armLowerControllerConfig(armLowerControllerConfig),
        m_motor(3), m_encoder(3), m_velocityPIDController(3), m_positionPIDController(3)
        {

    Motor motorBase(m_armLowerControllerConfig.baseMotor.pwmPin, m_armLowerControllerConfig.baseMotor.dirPin,
                    m_armLowerControllerConfig.baseMotor.inverted);

    Motor motorShoulder(m_armLowerControllerConfig.shoulderMotor.pwmPin, m_armLowerControllerConfig.shoulderMotor.dirPin,
                        m_armLowerControllerConfig.shoulderMotor.inverted);

    Motor motorElbow(m_armLowerControllerConfig.elbowMotor.pwmPin, m_armLowerControllerConfig.elbowMotor.dirPin,
                     m_armLowerControllerConfig.elbowMotor.inverted);

    m_motor[baseJoint]     = motorBase;
    m_motor[shoulderJoint] = motorBase;
    m_motor[elbowJoint]    = motorElbow

    m_encoder[baseJoint]     = PwmIn encoderBase(m_armLowerControllerConfig.baseEncoder.pwmPin);
    m_encoder[shoulderJoint] = PwmIn encoderShoulder(m_armLowerControllerConfig.shoulderEncoder.pwmPin);
    m_encoder[elbowJoint]    = PwmIn encoderElbow(m_armLowerControllerConfig.elbowEncoder.pwmPin);

    m_velocityPIDController[baseJoint] =
            PID(m_armLowerControllerConfig.baseVelocityPID.P, m_armLowerControllerConfig.baseVelocityPID.I,
                m_armLowerControllerConfig.baseVelocityPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

    m_velocityPIDController[shoulderJoint] =
            PID(m_armLowerControllerConfig.shoulderVelocityPID.P, m_armLowerControllerConfig.shoulderVelocityPID.I,
                m_armLowerControllerConfig.shoulderVelocityPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

    m_velocityPIDController[elbowJoint] =
            PID(m_armLowerControllerConfig.elbowVelocityPID.P, m_armLowerControllerConfig.elbowVelocityPID.I,
                m_armLowerControllerConfig.elbowVelocityPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

    m_positionPIDController[baseJoint] =
            PID(m_armLowerControllerConfig.basePositionPID.P, m_armLowerControllerConfig.basePositionPID.I,
                m_armLowerControllerConfig.basePositionPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

    m_positionPIDController[shoulderJoint] =
            PID(m_armLowerControllerConfig.shoulderPositionPID.P, m_armLowerControllerConfig.shoulderPositionPID.I,
                m_armLowerControllerConfig.shoulderPositionPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

    m_positionPIDController[elbowJoint] =
            PID(m_armLowerControllerConfig.elbowPositionPID.P, m_armLowerControllerConfig.elbowPositionPID.I,
                m_armLowerControllerConfig.elbowPositionPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

};

mbed_error_status_t ArmLowerController::setControlMode(t_controlMode controlMode) {
    return MBED_SUCCESS;
}

mbed_error_status_t ArmLowerController::setJointMotorSpeed(t_joint joint, float setSpeed) {
    if (m_controlMode != motorSpeed) {
        return MBED_ERROR_INVALID_ARGUMENT;
    }
}

mbed_error_status_t ArmLowerController::setJointVelocity(t_joint joint, float setVelocity) {
    if (m_controlMode != velocityPID) {
        return MBED_ERROR_INVALID_ARGUMENT;
    }
}

mbed_error_status_t ArmLowerController::setJointAngle(t_joint joint, float setAngle) {
    if (m_controlMode != positionPID) {
        return MBED_ERROR_INVALID_ARGUMENT;
    }
}

float ArmLowerController::getJointAngle(t_joint joint) {
    return m_encoder[joint].avgDutyCycle() * 360.0;
}

void ArmLowerController::update() {

}
