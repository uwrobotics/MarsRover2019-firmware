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

            // Limit Switch Configurations
            PinName limitSwitchTop;
            PinName limitSwitchBottom;
            float   calibrationDutyCycle;
            float   calibrationTimeoutSeconds;

            // PID Configuration
            PID::t_pidConfig positionPID;

            // PID Configuration
            unsigned int    maxEncoderPulses; // Should be an int
            unsigned int    maxDistanceInCM; // Should be an int
            float           centimetresPerPulse; // Unit is cm/pulse
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

        mbed_error_status_t setControlMode( t_elevatorControlMode controlMode );
        mbed_error_status_t setMotorDutyCycle(float percent);
        mbed_error_status_t setEncoderPositionPercent( float percent );
        mbed_error_status_t setPositionInCm(float centimeters);

        mbed_error_status_t runInitCalibration();

        t_elevatorControlMode getControlMode() const;

        int  getPositionEncoderPulses(); // Return encoder value
        int  getPositionCm(); // Return encoder transformed value into cm
        void update();

    private:

        void initializePID( void );
        
        t_elevatorControlMode   m_elevatorControlMode;
        t_elevatorConfig        m_elevatorConfig;
        DigitalIn               m_limitSwitchTop;
        DigitalIn               m_limitSwitchBottom;
    
        Motor   m_motor;
        QEI     m_encoder;

        PID     m_positionPIDController;

        Timer   timer;
};

#endif // ELEVATOR_CONTROLLER_H