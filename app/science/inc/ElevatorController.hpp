#ifndef ELEVATOR_CONTROLLER_H
#define ELEVATOR_CONTROLLER_H
// Controller for the Elevator moving the Auger

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"

// Methods of control
typedef enum t_elevatorControlMode {
    motorDutyCycle,
    positionPID

} t_elevatorControlMode;

// Motor info
typedef struct {
    PinName pwmPin;
    PinName dirPin;
    bool    inverted;

} t_motorConfig;

// Encoder info
typedef struct {
    PinName pwmPin;
    float   minEncoderVal; // = 0
    float   maxEncoderVal; // Min Limit
    bool    inverted;

} t_absoluteEncoderConfig;

// Limit switch info
// NOTE: there are two limit switches, at the top and bottom of the elevator
// TODO: VALIDATE - do we need the flags for top and bottom? Or can we get than info through the pin?
typedef struct {
    PinName topLSPin;       // Top limit switch pin
    PinName bottomLSPin;    // Bottom limit switch pin
    bool    top;            // If top limit switch is high, top = true
    bool    bottom;         // If bottom limit switch is high, bottom = true

} t_limitSwitchConfig;

// PID info
typedef struct {
    float P;
    float I;
    float D;
    float bias;

} t_pidConstants;

// General config
typedef struct{
    // Motor Configuration
    t_motorConfig motor;

    // Encoder Configuration
    t_absoluteEncoderConfig encoder;

    // PID Configuration
    t_pidConstants positionPID;

    // Limit Switch Configurations
    t_limitSwitchConfig limitSwitches;

    // PID Configuration
    float initPIDUpdateInterval;
    float PIDOutputMotorMinDutyCycle;
    float PIDOutputMotorMaxDutyCycle;

} t_elevatorControllerConfig;

class ElevatorController{
    private:
        void initializePID( void );
        
        t_elevatorControlMode       m_elevatorControlMode;
        t_elevatorControllerConfig  m_elevatorControllerConfig;
        t_limitSwitchConfig         m_limitSwitches;
    
        Motor m_motor;
        PwmIn m_encoder;

        PID m_positionPIDController;

        float m_inversionMultiplier;

        Timer timer;
    public:
        ElevatorController( t_elevatorControllerConfig  controllerConfig,
                            t_elevatorControlMode       controlMode );

        mbed_error_status_t  setControlMode( t_elevatorControlMode controlMode );
        mbed_error_status_t  setMotorSpeedPercent( float percent );
        mbed_error_status_t  setEncoderPositionPercent( float percent );
        mbed_error_status_t  maxLower(); // Wrapper for setPosition with lower encoder limit
        mbed_error_status_t  retract(); // Wrapper for setPosition with encoder = 0

        t_elevatorControlMode getControlMode() const;

        float   getPosition()           const; // Return encoder value
        float   getPositionPercent()    const; // Return encoder value as a percentage
        bool    getTopLimitSwitch()     const; // Value if top limit switch is triggered
        bool    getBottomLimitSwitch()  const; // Value is bottom limit switch is triggered
        void    update();
};

#endif // ELEVATOR_CONTROLLER_H