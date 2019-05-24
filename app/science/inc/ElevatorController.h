#ifndef ELEVATOR_CONTROLLER_H
#define ELEVATOR_CONTROLLER_H
// Controller for the Elevator moving the Auger

#include "mbed.h"
#include "Motor.h"
#include "QEI.h"
#include "PID.h"
#include "PinNames.h"

class ElevatorController{
    public:
        // General config
        typedef struct{
            // Motor Configuration
            Motor::t_motorConfig motor;

            // Encoder Configuration
            QEI::t_relativeEncoderConfig encoder;

            // PID Configuration
            PID::t_pidConfig positionPID;

            // Limit Switch Configurations
            PinName limitSwitchTop;
            PinName limitSwitchBottom;
            float   calibrationDutyCycle;
            float   calibrationTimeoutSeconds;

            // PID Configuration
            unsigned int    maxEncoderPulses; // Should be an int
            unsigned int    maxDistanceInCM; // Should be an int
            float           PIDOutputMotorMinDutyCycle;
            float           PIDOutputMotorMaxDutyCycle;

        } t_elevatorConfig;

        // Methods of control
        typedef enum t_elevatorControlMode {
            motorDutyCycle,
            positionPID

        } t_elevatorControlMode;

        ElevatorController( t_elevatorConfig        controllerConfig,
                            t_elevatorControlMode   controlMode );

        mbed_error_status_t  setControlMode( t_elevatorControlMode controlMode );
        mbed_error_status_t  setMotorSpeedPercent( float percent );
        mbed_error_status_t  setEncoderPositionPercent( float percent );
        mbed_error_status_t  setPositionInCM( float centimeter );

        mbed_error_status_t runInitCalibration();


        t_elevatorControlMode getControlMode() const;

        int     getPosition(); // Return encoder value
        int     getCurrentDistanceCM(); // Return encoder transformed value into cm
        void    update();

    private:
        void initializePID( void );
        
        t_elevatorControlMode   m_elevatorControlMode;
        t_elevatorConfig        m_elevatorConfig;
        DigitalIn               m_limitSwitchTop;
        DigitalIn               m_limitSwitchBottom;
    
        Motor   m_motor;
        QEI     m_encoder;

        PID m_positionPIDController;

        float m_inversionMultiplier;

        Timer timer;
};

#endif // ELEVATOR_CONTROLLER_H