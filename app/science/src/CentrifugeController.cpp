// Controller for the centrifuge

#include "../inc/CentrifugeController.hpp"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

CentrifugeController::CentrifugeController( t_centrifugeControllerConfig controllerConfig,
                                            t_centrifugeControlMode controlMode )
:   m_centrifugeControlMode( controlMode ),
    m_centrifugeControllerConfig( controllerConfig ),
    m_motor( controllerConfig.motor.pwmPin, controllerConfig.motor.dirPin, controllerConfig.motor.inverted ),
    m_encoder( controllerConfig.encoder.pwmPin ),
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

t_centrifugeControlMode CentrifugeController::getControlMode()
{
    return m_centrifugeControlMode;
}

// TODO: WRITE FUNCTION
float CentrifugeController::getAngle()
{
    return m_inversionMultiplier * 360.0f *( m_encoder.avgDutyCycle() - m_centrifugeControllerConfig.encoder.zeroAngleDutyCycle );
}

mbed_error_status_t CentrifugeController::setControlMode(t_centrifugeControlMode controlMode)
{
    m_centrifugeControlMode = controlMode;
    m_motor.speed( 0.0f );

    switch (m_centrifugeControlMode) {
        case motorDutyCycle:
            break;
        case positionPID:
            m_positionPIDController.reset();
            break;
    }

    timer.reset();

    return MBED_SUCCESS;
}

// TODO: VALIDATE FUNCTION
mbed_error_status_t CentrifugeController::setMotorSpeedPercent( float percent )
{
    if (m_centrifugeControlMode != motorDutyCycle) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    if ( percent > 1.0f || percent < -1.0f ) // If invalid argument, do nothing
    {
        percent = m_motor.read();
    }
    m_motor.speed( percent );
    return MBED_SUCCESS;
}

// Give PID controller an angle from 0-360?
mbed_error_status_t  setTubePosition( int tube_num )
{
    float rotation_angle = tube_num * 360 / 12;  //TODO: IS THERE AN OFFSET? ACCOUNT FOR AS A MEMBER FUNC?
    return MBED_SUCCESS;
}

// TODO: VALIDATE FUNCTION
void CentrifugeController::update()
{
    float interval = timer.read();
    timer.reset();

    switch( m_centrifugeControlMode ) {
        case motorDutyCycle:
            if ( m_motor.read() > 1.0f || m_motor.read() < 1.0f ) // If speed is > bounds, something is wrong
            {
                m_motor.speed(0.0f);
            }
            break;

        case positionPID:
            m_positionPIDController.setInterval( interval );
            m_positionPIDController.setProcessValue( getAngle() );
            m_motor.speed(m_positionPIDController.compute());
            break;
    }
}

void CentrifugeController::initializePID( void ) 
{
    // TODO: INPUT LIMITS? The motor is has circular motion without any limits? (0, 360) feels right but logically isn't correct
    m_positionPIDController.setInputLimits( m_centrifugeControllerConfig.encoder.minAngleDegrees * 360.0f, m_centrifugeControllerConfig.encoder.maxAngleDegrees * 360.0f );
    m_positionPIDController.setOutputLimits( m_centrifugeControllerConfig.PIDOutputMotorMinDutyCycle, m_centrifugeControllerConfig.PIDOutputMotorMaxDutyCycle );
    m_positionPIDController.setBias( m_centrifugeControllerConfig.positionPID.bias );
    m_positionPIDController.setMode( PID_AUTO_MODE );
}