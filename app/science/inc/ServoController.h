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

            // Probe servo configuration
            PinName probeServoPin;
            float probeUpPos, probeDownPos;

        } t_servoConfig;
        
        ServoController(t_servoConfig servoConfig);
                         
        void setFunnelUp(void);
        void setFunnelDown(void);

        void setProbeUp(void);
        void setProbeDown(void);

    private:

        t_servoConfig m_servoConfig;

        Servo m_funnelServo;
//        Servo m_probeServo;

        Ticker m_ticker;

        void setFunnelRest(void);
};


#endif // SERVO_CONTROLLER_H