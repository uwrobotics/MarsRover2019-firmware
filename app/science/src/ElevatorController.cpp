// Controller for the elevator
// NOTE: the convention established is that positive motor signals drive the elevator downwards
//          and negative signals drive the elevator upwards

#include "../inc/ElevatorController.hpp"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

ElevatorController::ElevatorController( t_elevatorControllerConfig  controllerConfig,
                                        t_elevatorControlMode       controlMode )
:   m_elevatorControlMode( controlMode ),
    m_elevatorControllerConfig( controllerConfig ),
    m_motor( controllerConfig.motor.pwmPin, controllerConfig.motor.dirPin, controllerConfig.motor.inverted ),
    m_encoder( controllerConfig.encoder.pwmPin ),
    m_limitSwitches( controllerConfig.limitSwitches.topLSPin, controllerConfig.limitSwitches.bottomLSPin, controllerConfig.limitSwitches.top, controllerConfig.limitSwitches.bottom ),
    m_positionPIDController( controllerConfig.positionPID.P, controllerConfig.positionPID.I, controllerConfig.positionPID.D, controllerConfig.initPIDUpdateInterval )
{
    if (controllerConfig.encoder.inverted) {
        m_inversionMultiplier = -1;
    }
    else {
        m_inversionMultiplier = 1;
    }

    initializePID();
    timer.start();

}

t_elevatorControlMode ElevatorController::getControlMode() const
{
    return m_elevatorControlMode;
}

// TODO: WRITE THIS FUNCTION - need to figure out how to access the encoder value
float ElevatorController::getPosition() const
{
    return ;
}

// Returns the position of the elevator as a percentage of the total length it can move
float ElevatorController::getPositionPercent() const
{
    return getPosition()/m_elevatorControllerConfig.encoder.maxEncoderVal;
}

// If the limit switch at the top of the elevator position is high
bool ElevatorController::getTopLimitSwitch() const
{
    return m_limitSwitches.top;
}

// If the limit switch at the bottom of the elevator position is high
bool ElevatorController::getBottomLimitSwitch() const
{
    return m_limitSwitches.bottom;
}

mbed_error_status_t ElevatorController::setControlMode( t_elevatorControlMode controlMode )
{
    m_elevatorControlMode = controlMode;
    m_motor.speed( 0.0f );

    switch (m_elevatorControlMode) {
        case motorDutyCycle:
            break;

        case positionPID:
            m_positionPIDController.reset();
            break;
    }

    timer.reset();

    return MBED_SUCCESS;
}

// Set the motor speed as a percentage of the maximum motor speed [-1.0, 1.0]
mbed_error_status_t ElevatorController::setMotorSpeedPercent(float percent)
{
    if (m_elevatorControlMode != motorDutyCycle) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    // We want double protection incase a sensor goes down or an encoder breaks
    // TODO: ISSUE - WHAT IF A SENSOR/ENCODER GOES OFFLINE AND IS ALWAYS SET TO HIGH?
    if (( getPosition() <= m_elevatorControllerConfig.encoder.minEncoderVal && percent < 0.0f ) ||
        ( getPosition() >= m_elevatorControllerConfig.encoder.maxEncoderVal && percent > 0.0f ) ||
        ( getTopLimitSwitch() && m_motor.read() < 0.0f)                                         ||
        ( getBottomLimitSwitch() && m_motor.read() > 0.0f))
    {
        percent = 0.0f;
    }

    m_motor.speed(percent);

    return MBED_SUCCESS;
}

// TODO: VALIDATE FUNCTION
mbed_error_status_t  ElevatorController::setEncoderPositionPercent( float percent )
{
    if( m_elevatorControlMode != positionPID ) { // Has to be in positionPID control
        return MBED_ERROR_INVALID_OPERATION;
    }

    // Do nothing
    if( percent < m_elevatorControllerConfig.encoder.minEncoderVal ||
        percent > m_elevatorControllerConfig.encoder.maxEncoderVal )
    {
        percent = getPosition() / m_elevatorControllerConfig.encoder.maxEncoderVal;
    }
    // Convert from percentage to encoder value - for PID
    percent *= m_elevatorControllerConfig.encoder.maxEncoderVal;
    m_positionPIDController.setSetPoint( percent );
    return MBED_SUCCESS;
}

mbed_error_status_t  ElevatorController::maxLower() // Wrapper for setPosition with lower encoder limit
{
    return setEncoderPositionPercent( 1.0f );
}

mbed_error_status_t  ElevatorController::retract() // Wrapper for setPosition with encoder = 0
{
    return setEncoderPositionPercent( 0.0f );
}


void ElevatorController::update() {
    float interval = timer.read();
    timer.reset();

    switch (m_elevatorControlMode) {
        case motorDutyCycle:
            if (( getPosition() <= m_elevatorControllerConfig.encoder.minEncoderVal && m_motor.read() < 0.0f )  ||
                ( getPosition() >= m_elevatorControllerConfig.encoder.maxEncoderVal && m_motor.read() > 0.0f )  ||
                ( getTopLimitSwitch() && m_motor.read() < 0.0f)                                                 ||
                ( getBottomLimitSwitch() && m_motor.read() > 0.0f))
            {
                m_motor.speed(0.0f);
            }
            break;

        case positionPID:
            m_positionPIDController.setInterval( interval );
            m_positionPIDController.setProcessValue( /*TODO: FILL FIELD*/ );
            m_motor.speed( m_positionPIDController.compute() );
            break;
    }
}

// TODO: is there any multiplier required on the input limits? - as there is within the ArmLowerController?
void ElevatorController::initializePID( void ) {
    m_positionPIDController.setInputLimits( m_elevatorControllerConfig.encoder.minEncoderVal, m_elevatorControllerConfig.encoder.maxEncoderVal );
    m_positionPIDController.setOutputLimits( m_elevatorControllerConfig.PIDOutputMotorMinDutyCycle, m_elevatorControllerConfig.PIDOutputMotorMaxDutyCycle );
    m_positionPIDController.setBias( m_elevatorControllerConfig.positionPID.bias );
    m_positionPIDController.setMode( PID_AUTO_MODE );
}