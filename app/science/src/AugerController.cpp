// Controller for the auger drill

#include "../inc/AugerController.hpp"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

AugerController::AugerController( t_augerControllerConfig controllerConfig )
:   m_augerControllerConfig( controllerConfig ),
    m_motor( controllerConfig.motor.pwmPin, controllerConfig.motor.dirPin, controllerConfig.motor.inverted ),
    m_encoder( controllerConfig.encoder.pwmPin )
{
    if( controllerConfig.encoder.inverted ) {
        m_inversionMultiplier = -1;
    }
    else {
        m_inversionMultiplier = 1;
    }
}

// Takes a float between -1 <= percent <= 1
mbed_error_status_t AugerController::setMotorSpeedPercent( float percent )
{
    m_motor.speed( percent );
    return MBED_SUCCESS;
}