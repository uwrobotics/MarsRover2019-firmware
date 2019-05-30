#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H
// Controller for the Auger

#include "mbed.h"
#include "Servo.h"
#include "PinNames.h"

class ServoController {

    public:

        typedef struct {

            // Funnel servo configuration
            PinName funnelServoPin;
            float funnelUpPos, funnelRestPos, funnelDownPos;

        } t_servoConfig;
        
        ServoController(t_servoConfig servoConfig);
                         
        void setFunnelUp(void);
        void setFunnelDown(void);

        bool isFunnelOpen(void);

    private:

        t_servoConfig m_servoConfig;

        Servo m_funnelServo;

        Ticker m_ticker;

        bool m_isFunnelOpen;

        void setFunnelRest(void);
};


#endif // SERVO_CONTROLLER_H