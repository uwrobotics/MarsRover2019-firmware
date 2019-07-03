#ifndef CENTRIFUGE_CONTROLLER_H
#define CENTRIFUGE_CONTROLLER_H
/* Controller for the Centrifuge/Turntable
 */

#include "mbed.h"
#include "Motor.h"
#include "QEI.h"
#include "PID.h"
#include "PinNames.h"

class CentrifugeController {

    public:

        typedef struct {

            // Motor Configuration
            Motor::t_motorConfig motor;

            // Encoder Configuration
            QEI::t_relativeEncoderConfig encoder;

            // Limit switch
            PinName limitSwitchPin;
            float   limitSwitchOffset;
            float   calibrationDutyCycle;
            float   calibrationTimeoutSeconds;
            float   spinningDutyCycle;

            // PID Configuration
            PID::t_pidConfig positionPID;

            unsigned int    maxEncoderPulsePerRev; // Should be an int
            float           PIDOutputMotorMinDutyCycle;
            float           PIDOutputMotorMaxDutyCycle;

        } t_centrifugeConfig;

        // Methods of control
        typedef enum t_centrifugeControlMode {
            motorDutyCycle,
            positionPID
        } t_centrifugeControlMode;

        explicit CentrifugeController( t_centrifugeConfig centrifugeConfig,
                                       t_centrifugeControlMode controlMode = motorDutyCycle );

        mbed_error_status_t setControlMode( t_centrifugeControlMode control );
        mbed_error_status_t setMotorDutyCycle(float dutyCycle);
        mbed_error_status_t setSpinning(bool spin);
        mbed_error_status_t setTubePosition( unsigned int tube_num ); // Range of [0-11]

        mbed_error_status_t runEndpointCalibration();

        t_centrifugeControlMode getControlMode();

        bool isSpinning();

        unsigned int    getTestTubeIndex(); // Return the test tube # that is currently under the auger
        float           getEncoderPulses(); // Return the # of encoder pulses within a single revolution
        float           getDutyCycle();
        void            update();

    private:

        void initializePID( void );

        t_centrifugeControlMode m_centrifugeControlMode;
        t_centrifugeConfig      m_centrifugeConfig;

        Motor       m_motor;
        QEI         m_encoder;
        DigitalIn   m_limitSwitch;

        PID m_positionPIDController;

        int m_encoderInversionMultiplier;
        bool m_isSpinning;

        Timer timer;

};

#endif // CENTRIFUGE_CONTROLLER_H