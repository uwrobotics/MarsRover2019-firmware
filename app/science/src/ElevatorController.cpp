// Controller for the elevator
// NOTE: the convention established is that positive motor signals drive the elevator downwards
//          and negative signals drive the elevator upwards

#include <algorithm>
#include "ElevatorController.h"
#include "../inc/ElevatorController.h"

ElevatorController::ElevatorController( ElevatorController::t_elevatorConfig        controllerConfig,
                                        ElevatorController::t_elevatorControlMode   controlMode )
:   m_elevatorControlMode( controlMode ),
    m_elevatorConfig( controllerConfig ),
    m_motor( controllerConfig.motor.pwmPin, controllerConfig.motor.dirPin, controllerConfig.motor.inverted ),
    m_encoder( controllerConfig.encoder ),
    m_limitSwitchTop( controllerConfig.limitSwitchTop),
    m_limitSwitchBottom( controllerConfig.limitSwitchBottom),
    m_positionPIDController( controllerConfig.positionPID.P, controllerConfig.positionPID.I, controllerConfig.positionPID.D, controllerConfig.positionPID.interval )
{
    if (controllerConfig.encoder.inverted) {
        m_encoderInversionMultiplier = -1;
    }
    else {
        m_encoderInversionMultiplier = 1;
    }

    initializePID();
    timer.start();
}

ElevatorController::t_elevatorControlMode ElevatorController::getControlMode() const
{
    return m_elevatorControlMode;
}

// Get position as encoder pulse count
int ElevatorController::getPositionEncoderPulses()
{
    return m_encoderInversionMultiplier * m_encoder.getPulses();
}

int ElevatorController::getPositionCm()
{
    return getPositionEncoderPulses() * m_elevatorConfig.centimetresPerPulse;
}

mbed_error_status_t ElevatorController::setControlMode( t_elevatorControlMode controlMode )
{
    switch (controlMode) {

        case motorDutyCycle:
            m_elevatorControlMode = motorDutyCycle;
            setMotorDutyCycle(0.0f);
            break;

        case positionPID:
            m_elevatorControlMode = positionPID;
            m_positionPIDController.reset();
            setPositionInCm(getPositionCm());
            break;

        default:
            return MBED_ERROR_CODE_INVALID_ARGUMENT;
    }

    timer.reset();

    return MBED_SUCCESS;
}

// Set the motor speed as a percentage of the maximum motor speed [-1.0, 1.0]
mbed_error_status_t ElevatorController::setMotorDutyCycle(float dutyCycle)
{
    if (m_elevatorControlMode != motorDutyCycle) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if ((m_limitSwitchTop.read() == 0 && dutyCycle < 0.0f) ||
        (m_limitSwitchBottom.read() == 0 && dutyCycle > 0.0f))
    {
        dutyCycle = 0.0f;
    }

    m_motor.setSpeed(dutyCycle);
    Serial pc(SERIAL_TX, SERIAL_RX);
    pc.printf("Set raw motor speed to %f\r\n", dutyCycle);

    return MBED_SUCCESS;
}

mbed_error_status_t ElevatorController::setPositionInCm(float centimeters)
{
    // Has to be in positionPID control
    if( m_elevatorControlMode != positionPID ) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    fmin(fmax(0.0, centimeters), m_elevatorConfig.maxDistanceCm);

    // Convert cm distance into encoder value
    m_positionPIDController.setSetPoint( centimeters / m_elevatorConfig.centimetresPerPulse);
    return MBED_SUCCESS;
}


void ElevatorController::update() {
    float interval = timer.read();
    timer.reset();

    switch (m_elevatorControlMode) {

        case motorDutyCycle:
            if ((m_limitSwitchTop.read() == 0 && m_motor.getSpeed() < 0.0f) ||
                (m_limitSwitchBottom.read() == 0 && m_motor.getSpeed() > 0.0f))
            {
                m_motor.setSpeed(0.0f);
                Serial pc(SERIAL_TX, SERIAL_RX);
                pc.printf("Motor limit hit with speed %din update loop, set motor speed to 0\r\n", m_motor.getSpeed());
            }
            break;

        case positionPID:
            m_positionPIDController.setInterval( interval );
            m_positionPIDController.setProcessValue( getPositionEncoderPulses() );
            m_motor.setSpeed( m_positionPIDController.compute() );
            break;
    }
}

// TODO: is there any multiplier required on the input limits? - as there is within the ArmLowerController?
void ElevatorController::initializePID( void ) {
    m_positionPIDController.setInputLimits( 0, m_elevatorConfig.maxEncoderPulses );
    m_positionPIDController.setOutputLimits( m_elevatorConfig.PIDOutputMotorMinDutyCycle, m_elevatorConfig.PIDOutputMotorMaxDutyCycle );
    m_positionPIDController.setBias( m_elevatorConfig.positionPID.bias );
    m_positionPIDController.setMode( PID_AUTO_MODE );
}

mbed_error_status_t ElevatorController::runEndpointCalibration() {

    t_elevatorControlMode prevControlMode = getControlMode();

    MBED_WARN_AND_RETURN_STATUS_ON_ERROR( setControlMode( motorDutyCycle ) );
    MBED_WARN_AND_RETURN_STATUS_ON_ERROR( setMotorDutyCycle(m_elevatorConfig.calibrationDutyCycle) );

    timer.reset();

    while ( m_limitSwitchTop.read() != 0 ) {
        if ( timer.read() > m_elevatorConfig.calibrationTimeoutSeconds ) {
            setControlMode(prevControlMode);
            return MBED_ERROR_TIME_OUT;
        }
    }

    m_encoder.reset();
    setMotorDutyCycle(0.0f);
    setControlMode(prevControlMode);

    return MBED_SUCCESS;
}