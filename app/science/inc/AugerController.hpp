#ifndef AUGER_CONTROLLER_H
#define AUGER_CONTROLLER_H
// Controller for the Auger

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"

// Methods of control
typedef enum t_augerControlMode {
    motorSpeed
} t_augerControlMode;

typedef struct{
    // Motor Configuration
    PinName augerMotor;
    float   maxMotorSpeed;

    // Motor Direction Configuration
    PinName augerMotorDir;

} t_augerControllerConfig;

class AugerController{
    private:
        t_augerControlMode      m_augerControlMode;
        t_augerControllerConfig m_augerControllerConfig;
    public:
        AugerController( t_augerControllerConfig m_augerControllerConfig,
                         t_augerControlMode      m_augerControlMode );
                         
        void setAugerMotorSpeed( float setSpeed );
};


#endif // AUGER_CONTROLLER_H