#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"
#include "PwmIn.h"
#include "PID.h"
#include "Motor.h"
#include "../inc/AugerController.h"
#include "../inc/CentrifugeController.h"
#include "../inc/ElevatorController.h"

//TODO: CHECK ALL CONFIGS - NEED TO VERIFY FROM SOMEONE WHO KNOWS THE WIRING
//TODO: MAP ENCODER PULSES TO DISTANCE FOR THE ELEVATOR
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
                .channelAPin = E_C_CH2,
                .indexPin    = E_C_INDEX,
                .pulsesPerRevolution = 360,
                .encoding = QEI::X4_ENCODING,
                .inverted = true
        },

        .limitSwitchPin = C_LS,

        .calibrationDutyCycle = 0.2f,
        .calibrationTimeoutSeconds = 7.0f,

        .positionPID = {
                .P    = 3.5f,
                .I    = 0.0f,
                .D    = 0.0f,
                .bias = 0.0f,
                .interval = 0.1f
        },

        .maxEncoderPulsePerRev = 360,
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
                .channelAPin = E_E_CH2,
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

        .maxEncoderPulses = 360,
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
            MBED_ASSERT_FAILURE();
        }
    }
    MBED_ASSERT_SUCCESS(elevatorController.setPositionInCM( setHeight ));
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
            MBED_ASSERT_FAILURE();
        }
    }
    MBED_ASSERT_SUCCESS(centrifugeController.setMotorSpeedPercent( spin ));


}

float handleSetCentrifugePosition(CANMsg *p_newMsg) {
    int tube_num = 0;
    *p_newMsg >> tube_num;

    CentrifugeController::t_centrifugeControlMode controlMode = centrifugeController.getControlMode();

    if( controlMode != CentrifugeController::positionPID ) {
        mbed_error_status_t status = centrifugeController.setControlMode( CentrifugeController::positionPID );
        if( status != MBED_SUCCESS ) {
            MBED_ASSERT_FAILURE();
        }
    }
    MBED_ASSERT_SUCCESS(centrifugeController.setTubePosition(tube_num));

}

void sendJetsonInfo() {
    
}

int main(void)
{
    pc.printf("Program Started\r\n\r\n");

    initCAN();

    elevatorController.runInitCalibration();
    centrifugeController.runInitCalibration();

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