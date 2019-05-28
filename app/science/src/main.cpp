#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"
#include "PwmIn.h"
#include "PID.h"
#include "Motor.h"
#include "AugerController.h"
#include "CentrifugeController.h"
#include "ElevatorController.h"

//TODO: FIGURE OUT HOW TO ROTATE THE CENTRIFUGE 15 DEGREES
const AugerController::t_augerConfig augerConfig = {
        .motor = {
                .pwmPin = MOTOR5,
                .dirPin = MOTOR5_DIR,
                .inverted = true
        }
};

const CentrifugeController::t_centrifugeConfig centrifugeConfig = {
        .motor = {
                .pwmPin = MOTOR6,
                .dirPin = MOTOR6_DIR,
                .inverted = true
        },

        .encoder = {
                .channelAPin = E_C_CH1,
                .channelBPin = E_C_CH2,
                .indexPin    = E_C_INDEX,
                .pulsesPerRevolution = 360,
                .encoding = QEI::X4_ENCODING,
                .inverted = true
        },

        .limitSwitchPin = C_LS,
        .limitSwitchOffset = 15.0f, // Limit switch is offset 15 degrees

        .calibrationDutyCycle = 0.2f,
        .calibrationTimeoutSeconds = 7.0f,

        .positionPID = {
                .P    = 3.5f,
                .I    = 0.0f,
                .D    = 0.0f,
                .bias = 0.0f,
                .interval = 0.1f
        },

        .maxEncoderPulsePerRev = 2112, // From counts per rev * gear ratio
        .PIDOutputMotorMinDutyCycle = -1.0f,
        .PIDOutputMotorMaxDutyCycle = 1.0f
};

const ElevatorController::t_elevatorConfig elevatorConfig = {

        .motor = {
                .pwmPin = MOTOR4,
                .dirPin = MOTOR4_DIR,
                .inverted = true
        },

        .encoder = {
                .channelAPin = E_E_CH1,
                .channelBPin = E_E_CH2,
                .indexPin    = E_E_INDEX,
                .pulsesPerRevolution = 360,
                .encoding = QEI::X4_ENCODING,
                .inverted = true
        },

        .limitSwitchTop = E_LS_T,
        .limitSwitchBottom = E_LS_B,

        .calibrationDutyCycle = 0.2f,
        .calibrationTimeoutSeconds = 7.0f,

        .positionPID = {
                .P    = 3.5f,
                .I    = 0.0f,
                .D    = 0.0f,
                .bias = 0.0f,
                .interval = 0.1f
        },

        .maxEncoderPulses = 160000, // Gotten from maxDistanceInCM / centimetresPerPulse
        .maxDistanceInCM = 26, // 10 inch range distance
        .centimetresPerPulse = 0.00016235795f, // Unit is cm/pulse
        .PIDOutputMotorMinDutyCycle = -1.0f,
        .PIDOutputMotorMaxDutyCycle = 1.0f
};

Serial             pc(SERIAL_TX, SERIAL_RX, ROVER_DEFAULT_BAUD_RATE);
CAN                can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg             rxMsg;
CANMsg             txMsg;

DigitalOut         ledErr(LED1);
DigitalOut         ledCAN(LED4);

AugerController         augerController( augerConfig );
CentrifugeController    centrifugeController( centrifugeConfig, CentrifugeController::motorDutyCycle );
ElevatorController      elevatorController( elevatorConfig, ElevatorController::positionPID );

Timer              canSendTimer;

void printCANMsg(CANMessage& msg) {
    pc.printf("  ID      = 0x%.3x\r\n", msg.id);
    pc.printf("  Type    = %d\r\n", msg.type);
    pc.printf("  Format  = %d\r\n", msg.format);
    pc.printf("  Length  = %d\r\n", msg.len);
    pc.printf("  Data    =");
    for(int i = 0; i < msg.len; i++)
        pc.printf(" 0x%.2X", msg.data[i]);
    pc.printf("\r\n");
}

enum scienceCommand {

    setElevatorHeight = ROVER_SCIENCE_CANID,
    setAugerSpeed,
    setCentrifugeSpinning,
    setCentrifugePosition,
    //setFunnelOpen,
    //setSensorMountPosition,

    firstCommand = setElevatorHeight,
    lastCommand  = setCentrifugePosition
    // lastCommand  = setSensorMountPosition

};

void initCAN() {
    can.filter(ROVER_SCIENCE_CANID, ROVER_CANID_FILTER_MASK, CANStandard);
}

float handleSetElevatorHeight(CANMsg *p_newMsg) {
    float setHeight = 0;
    *p_newMsg >> setHeight;

    ElevatorController::t_elevatorControlMode controlMode = elevatorController.getControlMode();

    if( controlMode != ElevatorController::positionPID ) {
        mbed_error_status_t status = elevatorController.setControlMode( ElevatorController::positionPID );
        if( status != MBED_SUCCESS ) {
            MBED_ASSERT( 0 );
        }
    }
    MBED_ASSERT_SUCCESS(elevatorController.setPositionInCm(setHeight));
}

float handleSetAugerSpeed(CANMsg *p_newMsg) {
    float speed = 0;
    *p_newMsg >> speed;
    MBED_ASSERT_SUCCESS(augerController.setMotorSpeedPercent( speed ));
}

float handleSetCentrifugeSpinning(CANMsg *p_newMsg) {
    bool spin = 0;
    *p_newMsg >> spin;

    CentrifugeController::t_centrifugeControlMode controlMode = centrifugeController.getControlMode();

    if( controlMode != CentrifugeController::motorDutyCycle ) {
        mbed_error_status_t status = centrifugeController.setControlMode( CentrifugeController::motorDutyCycle );
        if( status != MBED_SUCCESS ) {
            MBED_ASSERT( 0 );
        }
    }
    MBED_ASSERT_SUCCESS(centrifugeController.setMotorDutyCycle(spin));


}

float handleSetCentrifugePosition(CANMsg *p_newMsg) {
    int tube_num = 0;
    *p_newMsg >> tube_num;

    CentrifugeController::t_centrifugeControlMode controlMode = centrifugeController.getControlMode();

    if( controlMode != CentrifugeController::positionPID ) {
        mbed_error_status_t status = centrifugeController.setControlMode( CentrifugeController::positionPID );
        if( status != MBED_SUCCESS ) {
            MBED_ASSERT( 0 );
        }
    }
    MBED_ASSERT_SUCCESS(centrifugeController.setTubePosition(tube_num));

}

void processCANMsg(CANMsg *p_newMsg) {
    switch (p_newMsg->id) {

        case setElevatorHeight:
            handleSetElevatorHeight(p_newMsg);
            break;

        case setAugerSpeed:
            handleSetAugerSpeed(p_newMsg);
            break;

        case setCentrifugeSpinning:
            handleSetCentrifugeSpinning(p_newMsg);

        case setCentrifugePosition:
            handleSetCentrifugePosition(p_newMsg);
            break;

        default:
            pc.printf("Recieved unimplemented command\r\n");
            break;
    }
}

void sendJetsonInfo() {
    
}

int main(void)
{
    pc.printf("Program Started\r\n\r\n");

    initCAN();

    elevatorController.runInitCalibration();
    centrifugeController.runEndpointCalibration();

    canSendTimer.start();

    while (1) {

        if (can.read(rxMsg)) {
            processCANMsg(&rxMsg);
            rxMsg.clear();
            ledCAN = !ledCAN;
        }

        if (canSendTimer.read() > 0.1) {
            sendJetsonInfo();
            canSendTimer.reset();
        }

        elevatorController.update();
        centrifugeController.update();

    }
}