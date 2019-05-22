#ifndef CENTRIFUGE_CONTROLLER_H
#define CENTRIFUGE_CONTROLLER_H
// Controller for the Centrifuge/Turntable

#include "mbed.h"
#include "Motor.h"
#include "PwmIn.h"
#include "PID.h"
#include "PinNames.h"

// Motor information
typedef struct {
    PinName pwmPin;
    PinName dirPin;
    bool inverted;
} t_motorConfig;

typedef struct {
    PinName pwmPin;
    bool inverted;
} t_absoluteEncoderConfig;

// TODO: VALIDATE - same Q in elevator controller - do we need a flag for the switch, or just get the value from the pin?
typedef struct {
    PinName LSPin;
    bool    limitSwitch;    // If top limit switch is high, top = true

} t_limitSwitchConfig;

typedef struct {
    float P;
    float I;
    float D;
    float bias;
} t_pidConstants;

// Methods of control
typedef enum t_centrifugeControlMode {
    motorDutyCycle,
    positionPID
} t_centrifugeControlMode;

typedef struct{
    // Motor Configuration
    t_motorConfig motor;

    // Encoder Configuration
    t_absoluteEncoderConfig encoder;

    // PID Configuration
    t_pidConstants positionPID;

    float initPIDUpdateInterval;
    float PIDOutputMotorMinDutyCycle;
    float PIDOutputMotorMaxDutyCycle;

    // Max and min PWM signals for motor unit speed
    const float PWM_MIN_SIGNAL = -1.0;
    const float PWM_MAX_SIGNAL = 1.0;
} t_centrifugeControllerConfig;

class CentrifugeController{
    private:
        void initializePID( void );

        t_centrifugeControlMode         m_centrifugeControlMode;
        t_centrifugeControllerConfig    m_centrifugeControllerConfig;

        Motor m_motor;
        PwmIn m_encoder;

        PID m_positionPIDController;

        const int   LIMIT_SWITCH_ANGLE_OFFSET = 15; // Value in degrees
        float       m_inversionMultiplier;

        Timer timer;
    public:
        explicit CentrifugeController( t_centrifugeControllerConfig controllerConfig,
                                       t_centrifugeControlMode      controlMode = motorDutyCycle );

        mbed_error_status_t  setControlMode( t_centrifugeControlMode control );
        mbed_error_status_t  setMotorSpeedPercent( float percent );

        // tube_num within range [0-11] - places tube of # tube_num under the Auger output mouth
        mbed_error_status_t  setTubePosition( int tube_num );

        t_centrifugeControlMode getControlMode();

        float getAngle(); // Returns the angle tube_0 is from under Auger mouth
        void  resetToTube0(); // Restores initial position of centrifuge (ie. tube_0 under Auger mouth)
        void  update();
};

#endif // CENTRIFUGE_CONTROLLER_H