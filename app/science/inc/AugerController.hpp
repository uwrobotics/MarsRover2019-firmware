#ifndef AUGER_CONTROLLER_H
#define AUGER_CONTROLLER_H
// Controller for the Auger

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"

// Methods of control
typedef struct {
    PinName pwmPin;
    PinName dirPin;
    float   maxMotorSpeed;
    bool    inverted;
} t_motorConfig;

typedef struct {
    PinName pwmPin;
    bool inverted;
} t_absoluteEncoderConfig;

typedef struct{
    // Motor Configuration
    t_motorConfig motor;

    // Encoder Configuration
    t_absoluteEncoderConfig encoder;

} t_augerControllerConfig;

class AugerController{
    private:
        t_augerControllerConfig m_augerControllerConfig;

        Motor m_motor;
        PwmIn m_encoder;

        float m_inversionMultiplier;
        
    public:
        AugerController( t_augerControllerConfig controllerConfig );
                         
        mbed_error_status_t setMotorSpeedPercent( float percent );
};


#endif // AUGER_CONTROLLER_H