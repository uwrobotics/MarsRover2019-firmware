// Controller for the centrifuge

#include "CentrifugeController.h"

CentrifugeController::CentrifugeController( CentrifugeController::t_centrifugeConfig        centrifugeConfig,
                                            CentrifugeController::t_centrifugeControlMode   controlMode ):
    m_centrifugeControlMode( controlMode ),
    m_centrifugeConfig( centrifugeConfig ),
    m_motor( centrifugeConfig.motor ),
    m_encoder( centrifugeConfig.encoder ),
    m_limitSwitch(centrifugeConfig.limitSwitchPin ),
    m_positionPIDController( centrifugeConfig.positionPID.P, centrifugeConfig.positionPID.I, centrifugeConfig.positionPID.D, centrifugeConfig.positionPID.interval )
{

    if (centrifugeConfig.encoder.inverted) {
        m_encoderInversionMultiplier = -1;
    }
    else {
        m_encoderInversionMultiplier = 1;
    }

    m_isSpinning = false;

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
    return static_cast<int>( getEncoderPulses() / m_centrifugeConfig.maxEncoderPulsePerRev * 11.9f );
}

// Get the current encoder value
float CentrifugeController::getEncoderPulses()
{
    return m_encoderInversionMultiplier * fmodf(m_encoder.getPulses(), m_centrifugeConfig.maxEncoderPulsePerRev);
}

mbed_error_status_t CentrifugeController::setControlMode(CentrifugeController::t_centrifugeControlMode controlMode)
{
    switch (controlMode) {

        case motorDutyCycle:
            m_centrifugeControlMode = motorDutyCycle;
            setMotorDutyCycle(0.0f);
            break;

        case positionPID:
            m_positionPIDController.reset();
            m_centrifugeControlMode = positionPID;
            setTubePosition(0);
            break;

        default:
            return MBED_ERROR_CODE_INVALID_ARGUMENT;
    }

    timer.reset();

    return MBED_SUCCESS;
}

// Set the motor speed in terms of % of the total speed
mbed_error_status_t CentrifugeController::setMotorDutyCycle(float dutyCycle)
{
    if (m_centrifugeControlMode != motorDutyCycle) {
        return MBED_ERROR_INVALID_OPERATION;
    }

    m_motor.setSpeed( dutyCycle );
    return MBED_SUCCESS;
}

// Set the PID in terms of the test tube number
mbed_error_status_t CentrifugeController::setTubePosition( unsigned int tube_num )
{
    if (getControlMode() != positionPID) {
        setControlMode(positionPID);
    }

    // Might shift the placement by one test tube due to rounding when fetching current tube #
    if( tube_num > 11 ){
        tube_num = 11;
    }

    m_positionPIDController.setSetPoint( (tube_num / 12.0f * m_centrifugeConfig.maxEncoderPulsePerRev) + m_centrifugeConfig.limitSwitchOffset );

    return MBED_SUCCESS;
}

void CentrifugeController::update()
{
    float interval = timer.read();
    timer.reset();

    switch( m_centrifugeControlMode ) {

        case motorDutyCycle:

            if ( m_motor.getSpeed() > 1.1f || m_motor.getSpeed() < -1.1f ) // If speed is > bounds, something is wrong
            {
                m_motor.setSpeed(0.0f);
            }

            break;

        case positionPID:

            m_positionPIDController.setInterval( interval );
            m_positionPIDController.setProcessValue( getEncoderPulses() );
            m_motor.setSpeed(m_positionPIDController.compute());

            break;
    }
}

void CentrifugeController::initializePID( void ) 
{
    // Input limits are encoder pulses
    m_positionPIDController.setInputLimits( 0 , m_centrifugeConfig.maxEncoderPulsePerRev );
    m_positionPIDController.setOutputLimits( m_centrifugeConfig.PIDOutputMotorMinDutyCycle, m_centrifugeConfig.PIDOutputMotorMaxDutyCycle );
    m_positionPIDController.setBias( m_centrifugeConfig.positionPID.bias );
    m_positionPIDController.setDeadZoneError( 0.02 );
    m_positionPIDController.setMode( PID_AUTO_MODE );
}

mbed_error_status_t CentrifugeController::runEndpointCalibration() {

    t_centrifugeControlMode prevControlMode = getControlMode();

    setControlMode( motorDutyCycle );
    setMotorDutyCycle(m_centrifugeConfig.calibrationDutyCycle);

    timer.reset();

    while ( m_limitSwitch.read() != 0 ) {
        if ( timer.read() > m_centrifugeConfig.calibrationTimeoutSeconds ) {
            setMotorDutyCycle(0.0f);
            setControlMode(prevControlMode);
            return MBED_ERROR_TIME_OUT;
        }
    }

    m_encoder.reset();
    setMotorDutyCycle(0.0f);
    setControlMode(prevControlMode);

    return MBED_SUCCESS;
}

mbed_error_status_t CentrifugeController::setSpinning(bool spin) {

    if ( getControlMode() != CentrifugeController::motorDutyCycle ) {
        setControlMode( CentrifugeController::motorDutyCycle);
    }

    if (spin) {
        setMotorDutyCycle(m_centrifugeConfig.spinningDutyCycle);
        m_isSpinning = true;
    }

    else {
        setMotorDutyCycle(0.0f);
        m_isSpinning = false;
    }

    return MBED_SUCCESS;
}

bool CentrifugeController::isSpinning() {
    return m_isSpinning;
}

float CentrifugeController::getDutyCycle() {
    return m_motor.getSpeed();
}
