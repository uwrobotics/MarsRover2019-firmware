// Controller for the auger drill

#include "../inc/AugerController.h"

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

AugerController::AugerController( AugerController::t_augerConfig controllerConfig )
:   m_augerConfig( controllerConfig ),
    m_motor( controllerConfig.motor )
{}

mbed_error_status_t AugerController::setMotorDutyCycle(float percent)
{
    m_motor.setSpeed( percent );
    return MBED_SUCCESS;
}