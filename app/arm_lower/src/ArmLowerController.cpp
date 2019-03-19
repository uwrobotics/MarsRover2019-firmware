#include "ArmLowerController.h"

// CLASS

class ArmLowerController {

public:

    ArmLowerController::ArmLowerController(t_armLowerControllerConfig armLowerControllerConfig, t_controlMode controlMode) :
        m_armLowerControllerConfig(armLowerControllerConfig) {

        ArmLowerController(controlMode);
    }

    ArmLowerController::ArmLowerController(t_controlMode controlMode) : m_controlMode(controlMode) {

        m_motor[t_joint.baseJoint] = Motor(m_armLowerControllerConfig.baseMotor.pwmIn,
                m_armLowerControllerConfig.baseMotor.dirPin, m_armLowerControllerConfig.baseMotor.inverted);
        m_motor[t_joint.shoulderJoint] = Motor(m_armLowerControllerConfig.shoulderMotor.pwmPin,
                m_armLowerControllerConfig.shoulderMotor.dirPin, m_armLowerControllerConfig.shoulderMotor.inverted);
        m_motor[t_joint.elbowJoint] = Motor(m_armLowerControllerConfig.elbowMotor.pwmPin,
                m_armLowerControllerConfig.elbowMotor.dirPin, m_armLowerControllerConfig.elbowMotor.inverted);

        m_encoder[t_joint.baseJoint] = PwmIn(m_armLowerControllerConfig.baseEncoder.pwmPin);
        m_encoder[t_joint.shoulderJoint] = PwmIn(m_armLowerControllerConfig.shoulderEncoder.pwmPin);
        m_encoder[t_joint.elbowJoint] = PwmIn(m_armLowerControllerConfig.elbowEncoder.pwmPin);

        m_velocityPIDController[t_joint.baseJoint] =
                PID(m_armLowerControllerConfig.baseVelocityPID.P, m_armLowerControllerConfig.baseVelocityPID.I,
                    m_armLowerControllerConfig.baseVelocityPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

        m_velocityPIDController[t_joint.shoulderJoint] =
                PID(m_armLowerControllerConfig.shoulderVelocityPID.P, m_armLowerControllerConfig.shoulderVelocityPID.I,
                    m_armLowerControllerConfig.shoulderVelocityPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

        m_velocityPIDController[t_joint.elbowJoint] =
                PID(m_armLowerControllerConfig.elbowVelocityPID.P, m_armLowerControllerConfig.elbowVelocityPID.I,
                    m_armLowerControllerConfig.elbowVelocityPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

        m_positionPIDController[t_joint.baseJoint] =
                PID(m_armLowerControllerConfig.basePositionPID.P, m_armLowerControllerConfig.basePositionPID.I,
                    m_armLowerControllerConfig.basePositionPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

        m_positionPIDController[t_joint.shoulderJoint] =
                PID(m_armLowerControllerConfig.shoulderPositionPID.P, m_armLowerControllerConfig.shoulderPositionPID.I,
                    m_armLowerControllerConfig.shoulderPositionPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

        m_positionPIDController[t_joint.elbowJoint] =
                PID(m_armLowerControllerConfig.elbowPositionPID.P, m_armLowerControllerConfig.elbowPositionPID.I,
                    m_armLowerControllerConfig.elbowPositionPID.D, m_armLowerControllerConfig.PIDUpdateInterval);

    };

    mbed_error_status_t ArmLowerController::setControlMode(t_controlMode controlMode);

    mbed_error_status_t ArmLowerController::setJointMotorSpeed(t_joint joint, float setSpeed) {
        if (m_controlMode != t_controlMode.motorSpeed) {
            return INVALID_ARGUMENT;
        }
    }

    mbed_error_status_t ArmLowerController::setJointVelocity(t_joint joint, float setVelocity) {
        if (m_controlMode != t_controlMode.velocityPID) {
            return INVALID_ARGUMENT;
        }
    }

    mbed_error_status_t ArmLowerController::setJointAngle(t_joint joint, float setAngle) {
        if (m_controlMode != t_controlMode.positionPID) {
            return INVALID_ARGUMENT;
        }
    }

    float ArmLowerController::getJointAngle(t_joint joint) {
        return m_encoder[joint].avgDutyCycle() * 360.0;
    }

    void ArmLowerController::update()

private:


};

#endif // ARM_LOWER_CONTROLLER_H
