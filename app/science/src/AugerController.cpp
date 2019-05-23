// Controller for the auger drill

#include <AugerController.h>
#include "../inc/AugerController.h"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

AugerController::AugerController( AugerController::t_augerConfig controllerConfig )
:   m_augerConfig( controllerConfig ),
    m_motor( controllerConfig.motor ),
    m_encoder( controllerConfig.encoder )
{}

mbed_error_status_t AugerController::setMotorSpeedPercent( float percent )
{
    m_motor.speed( percent );
    return MBED_SUCCESS;
}