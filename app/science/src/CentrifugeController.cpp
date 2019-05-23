// Controller for the centrifuge

#include <CentrifugeController.h>
#include "../inc/CentrifugeController.h"

CentrifugeController::CentrifugeController( CentrifugeController::t_centrifugeConfig        controllerConfig,
                                            CentrifugeController::t_centrifugeControlMode   controlMode )
:   m_centrifugeControlMode( controlMode ),
    m_centrifugeConfig( controllerConfig ),
    m_motor( controllerConfig.motor ),
    m_encoder( controllerConfig.encoder ),
    m_limitSwitch(controllerConfig.limitSwitchPin),
    m_positionPIDController( controllerConfig.positionPID.P, controllerConfig.positionPID.I, controllerConfig.positionPID.D, controllerConfig.positionPID.interval )
{
    initializePID();
    timer.start();
}

CentrifugeController::t_centrifugeControlMode CentrifugeController::getControlMode()
{
    return m_centrifugeControlMode;
}

// Get the current test tube # that is under the auger - note, this rounds down
unsigned int CentrifugeController::getTestTubeIndex()
{
    return static_cast<int>( getEncoderPulses()/m_centrifugeConfig.maxEncoderPulsePerRev * 12 );
}

// Get the current encoder value
float CentrifugeController::getEncoderPulses()
{
    return m_encoder.getPulses() % m_centrifugeConfig.maxEncoderPulsePerRev;
}

mbed_error_status_t CentrifugeController::setControlMode(CentrifugeController::t_centrifugeControlMode controlMode)
{
    m_centrifugeControlMode = controlMode;
    m_motor.speed( 0.0f );

    switch (m_centrifugeControlMode) {
        case motorDutyCycle:
            break;
        case positionPID:
            m_positionPIDController.reset();
            break;
        default:
            return MBED_ERROR_CODE_INVALID_ARGUMENT;
    }

    timer.reset();

    return MBED_SUCCESS;
}

// Set the motor speed in terms of % of the total speed
mbed_error_status_t CentrifugeController::setMotorSpeedPercent( float percent )
{
    if (m_centrifugeControlMode != motorDutyCycle) {
        return MBED_ERROR_INVALID_OPERATION;
    }
    m_motor.speed( percent );
    return MBED_SUCCESS;
}

// Set the PID in terms of the test tube number
mbed_error_status_t  CentrifugeController::setTubePosition( unsigned int tube_num )
{
    // Might shift the placement by one test tube due to rounding when fetching current tube #
    if( tube_num > 11 ){
        tube_num = getTestTubeIndex();
    }
    m_positionPIDController.setSetPoint( tube_num / 11 * m_centrifugeConfig.maxEncoderPulsePerRev );
    return MBED_SUCCESS;
}

void CentrifugeController::update()
{
    float interval = timer.read();
    timer.reset();

    switch( m_centrifugeControlMode ) {
        case motorDutyCycle:
            if ( m_motor.read() > 1.1f || m_motor.read() < 1.1f ) // If speed is > bounds, something is wrong
            {
                m_motor.speed(0.0f);
            }
            break;

        case positionPID:
            m_positionPIDController.setInterval( interval );
            m_positionPIDController.setProcessValue( getEncoderPulses() );
            m_motor.speed(m_positionPIDController.compute());
            break;
    }
}

void CentrifugeController::initializePID( void ) 
{
    // Input limits are encoder pulses
    m_positionPIDController.setInputLimits( 0 , m_centrifugeConfig.maxEncoderPulsePerRev );
    m_positionPIDController.setOutputLimits( m_centrifugeConfig.PIDOutputMotorMinDutyCycle, m_centrifugeConfig.PIDOutputMotorMaxDutyCycle );
    m_positionPIDController.setBias( m_centrifugeConfig.positionPID.bias );
    m_positionPIDController.setMode( PID_AUTO_MODE );
}

mbed_error_status_t CentrifugeController::runInitCalibration() {

    MBED_ASSERT_SUCCESS_RETURN_ERROR( setControlMode( motorDutyCycle ) );
    MBED_ASSERT_SUCCESS_RETURN_ERROR( setMotorSpeedPercent( m_centrifugeConfig.calibrationDutyCycle ) );

    timer.reset();

    while ( m_limitSwitch.read() == 0 ) {
        if ( timer.read() > m_centrifugeConfig.calibrationTimeoutSeconds ) {
            return MBED_ERROR_TIME_OUT;
        }
    }

    //TODO: ROTATE 15 DEGREES TO LINE UP TUBE 0

    m_encoder.reset();

    return MBED_SUCCESS;
}