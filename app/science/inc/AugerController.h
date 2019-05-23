#ifndef AUGER_CONTROLLER_H
#define AUGER_CONTROLLER_H
// Controller for the Auger

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"

class AugerController{
    public:
        typedef struct{
            // Motor Configuration
            Motor::t_motorConfig motor;

            // Encoder Configuration
            PwmIn::t_encoderConfig encoder;

        } t_augerConfig;
        
        AugerController( t_augerConfig controllerConfig );
                         
        mbed_error_status_t setMotorSpeedPercent( float percent );
    private:
        t_augerConfig m_augerConfig;

        Motor m_motor;
        PwmIn m_encoder;

        float m_inversionMultiplier;  
};


#endif // AUGER_CONTROLLER_H