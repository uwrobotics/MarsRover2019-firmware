#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"
#include "PwmIn.h"
#include "PID.h"
#include "Motor.h"
#include "AugerController.h"
#include "CentrifugeController.h"
#include "ElevatorController.h"
#include "ServoController.h"

//TODO: FIGURE OUT HOW TO ROTATE THE CENTRIFUGE 15 DEGREES
const AugerController::t_augerConfig augerConfig = {
        .motor = {
                .pwmPin = MOTOR_A,
                .dirPin = MOTOR_A_DIR,
                .inverted = false,
                .freqInHz = MOTOR_DEFAULT_FREQUENCY_HZ,
                .limit = 1.0
        }
};

const CentrifugeController::t_centrifugeConfig centrifugeConfig = {
        .motor = {
                .pwmPin = MOTOR_C,
                .dirPin = MOTOR_C_DIR,
                .inverted = false,
                .freqInHz = MOTOR_DEFAULT_FREQUENCY_HZ,
                .limit = 0.4
        },

        .encoder = {
                .channelAPin = ENC_C_CH1,
                .channelBPin = ENC_C_CH2,
                .indexPin    = ENC_C_INDEX,
                .pulsesPerRevolution = 211,
                .encoding = QEI::X4_ENCODING,
                .inverted = true
        },

        .limitSwitchPin = C_LS,
        .limitSwitchOffset = 15.0f, // Limit switch is offset 15 degrees

        .calibrationDutyCycle = 0.25f,
        .calibrationTimeoutSeconds = 7.0f,
        .spinningDutyCycle = 0.4f,

        .positionPID = {
                .P    = 8.5f,
                .I    = 0.0f,
                .D    = 0.0f,
                .bias = 0.0f,
                .interval = 0.1f
        },

        .maxEncoderPulsePerRev = 2112, // From counts per rev * gear ratio
        .PIDOutputMotorMinDutyCycle = -0.3f,
        .PIDOutputMotorMaxDutyCycle = 0.3f
};

const ElevatorController::t_elevatorConfig elevatorConfig = {

        .motor = {
                .pwmPin = MOTOR_E,
                .dirPin = MOTOR_E_DIR,
                .inverted = false,
                .freqInHz = MOTOR_DEFAULT_FREQUENCY_HZ,
                .limit = 1.0
        },

        .encoder = {
                .channelAPin = ENC_E_CH1,
                .channelBPin = ENC_E_CH2,
                .indexPin    = ENC_E_INDEX,
                .pulsesPerRevolution = 360,
                .encoding = QEI::X4_ENCODING,
                .inverted = true
        },

        .limitSwitchTop = E_LS_T,
        .limitSwitchBottom = E_LS_B,

        .calibrationDutyCycle = -0.2f,
        .calibrationTimeoutSeconds = 30.0f,

        .positionPID = {
                .P    = 3.5f,
                .I    = 0.0f,
                .D    = 0.0f,
                .bias = 0.0f,
                .interval = 0.1f
        },

        .maxEncoderPulses = 467352, // Gotten from maxDistanceCm / centimetresPerPulse
        .maxDistanceCm = 16, // 6.5 inch range distance
        .centimetresPerPulse = 0.00003532669f, // Unit is cm/pulse
        .PIDOutputMotorMinDutyCycle = -0.5f,
        .PIDOutputMotorMaxDutyCycle = 1.0f
};

const ServoController::t_servoConfig servoConfig {
    .funnelServoPin = SERVO_P,
    .funnelUpPos = 0.8,
    .funnelRestPos = 0.55,
    .funnelDownPos = 0.2
};

Serial             pc(SERIAL_TX, SERIAL_RX, ROVER_DEFAULT_BAUD_RATE);
CAN                can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg             rxMsg;
CANMsg             txMsg;

DigitalOut         ledErr(LED1);
DigitalOut         ledCAN(LED4);

AugerController         augerController( augerConfig );
CentrifugeController    centrifugeController( centrifugeConfig );
ElevatorController      elevatorController( elevatorConfig );
ServoController         servoController( servoConfig );

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

    setElevatorControlMode = ROVER_SCIENCE_CANID,
    setElevatorMotion,
    setAugerDutyCycle,
    setCentrifugeControlMode,
    setCentrifugeDutyCycle,
    setCentrifugeSpinning,
    setCentrifugePosition,
    setFunnelOpen,
    setProbeDeployed
};

enum jetsonFeedback {

    augerHeight = ROVER_JETSON_START_CANID_MSG_SCIENCE,
    augerSpeed,
    centrifugeSpinning,
    centrifugeSpeed,
    centrifugePosition,
    funnelStatus
};

void initCAN() {
    can.filter(ROVER_SCIENCE_CANID, ROVER_CANID_FILTER_MASK, CANStandard);
}

ElevatorController::t_elevatorControlMode handleSetElevatorControlMode(CANMsg *p_newMsg) {
    ElevatorController::t_elevatorControlMode controlMode;
    *p_newMsg >> controlMode;

    MBED_WARN_ON_ERROR(elevatorController.setControlMode(controlMode));

    return controlMode;
}


float handleSetElevatorMotion(CANMsg *p_newMsg) {
    float motionData = 0;
    *p_newMsg >> motionData;

    ElevatorController::t_elevatorControlMode controlMode = elevatorController.getControlMode();

    switch (controlMode) {
        case ElevatorController::motorDutyCycle:
            MBED_WARN_ON_ERROR(elevatorController.setMotorDutyCycle(motionData));
            pc.printf("Set elevator motor duty cycle to %f\r\n", motionData);
            break;
        case ElevatorController::positionPID:
            MBED_WARN_ON_ERROR(elevatorController.setPositionInCm(motionData));
            pc.printf("Set elevator position to %f cm\r\n", motionData);
            break;
    }

    return motionData;
}

float handleSetAugerDutyCycle(CANMsg *p_newMsg) {
    float dutyCycle = 0;
    *p_newMsg >> dutyCycle;

    MBED_WARN_ON_ERROR(augerController.setMotorDutyCycle(dutyCycle));

    return dutyCycle;
}

CentrifugeController::t_centrifugeControlMode handleSetCentrifugeControlMode(CANMsg *p_newMsg) {
    CentrifugeController::t_centrifugeControlMode controlMode;
    *p_newMsg >> controlMode;

    MBED_WARN_ON_ERROR(centrifugeController.setControlMode(controlMode));

    return controlMode;
}

float handleSetCentrifugeDutyCycle(CANMsg *p_newMsg) {
    float dutyCycle = 0;
    *p_newMsg >> dutyCycle;

    MBED_WARN_ON_ERROR(centrifugeController.setMotorDutyCycle( dutyCycle ));

    pc.printf("Set cent DC to %f", dutyCycle);

    return dutyCycle;
}

bool handleSetCentrifugeSpinning(CANMsg *p_newMsg) {
    bool spin = false;
    *p_newMsg >> spin;

    centrifugeController.setSpinning(spin);

    return spin;
}

int handleSetCentrifugePosition(CANMsg *p_newMsg) {
    int tube_num = 0;
    *p_newMsg >> tube_num;

    CentrifugeController::t_centrifugeControlMode controlMode = centrifugeController.getControlMode();

    if( controlMode != CentrifugeController::positionPID ) {
        MBED_WARN_ON_ERROR(centrifugeController.setControlMode( CentrifugeController::positionPID ));
    }

    pc.printf("Tube num %d\r\n", tube_num);

    MBED_WARN_ON_ERROR(centrifugeController.setTubePosition(tube_num));

    return tube_num;
}

bool handleSetFunnelOpen(CANMsg *p_newMsg) {
    bool open = false;
    *p_newMsg >> open;

    if (open) {
        servoController.setFunnelDown();
    }
    else {
        servoController.setFunnelUp();
    }

    return open;
}

void processCANMsg(CANMsg *p_newMsg) {
    switch (p_newMsg->id) {

        case setElevatorControlMode:
            pc.printf("Set elevator control mode to %d\r\n", handleSetElevatorControlMode(p_newMsg));
            break;

        case setElevatorMotion:
            handleSetElevatorMotion(p_newMsg);
            break;

        case setAugerDutyCycle:
            handleSetAugerDutyCycle(p_newMsg);
            break;

        case setCentrifugeControlMode:
            pc.printf("Set centrifuge control mode to %d\r\n", handleSetElevatorControlMode(p_newMsg));
            break;

        case setCentrifugeDutyCycle:
            pc.printf("Setting centrifuge duty cycle to %f\r\n", handleSetCentrifugeDutyCycle(p_newMsg));
            break;

        case setCentrifugeSpinning:
            handleSetCentrifugeSpinning(p_newMsg);
            break;

        case setCentrifugePosition:
            pc.printf("Set centrifuge position to %d\r\n", handleSetCentrifugePosition(p_newMsg));
            break;

        case setFunnelOpen:
            handleSetFunnelOpen(p_newMsg);
            break;

        case setProbeDeployed:
            break;

        default:
            pc.printf("Recieved unimplemented command\r\n");
            break;
    }
}

void sendJetsonInfo() {
    CANMsg txMsg(0);

    txMsg.clear();
    txMsg.id = augerHeight;
    txMsg << elevatorController.getPositionCm();
    MBED_ASSERT_WARN(can.write(txMsg) == true);

    txMsg.clear();
    txMsg.id = augerSpeed;
    txMsg << augerController.getDutyCycle();
    MBED_ASSERT_WARN(can.write(txMsg) == true);

    txMsg.clear();
    txMsg.id = centrifugeSpinning;
    txMsg << centrifugeController.isSpinning();
    MBED_ASSERT_WARN(can.write(txMsg) == true);

    txMsg.clear();
    txMsg.id = centrifugeSpeed;
    txMsg << centrifugeController.getDutyCycle();
    MBED_ASSERT_WARN(can.write(txMsg) == true);

    txMsg.clear();
    txMsg.id = centrifugePosition;
    txMsg << centrifugeController.getTestTubeIndex();
    MBED_ASSERT_WARN(can.write(txMsg) == true);

    txMsg.clear();
    txMsg.id = funnelStatus;
    txMsg << servoController.isFunnelOpen();
    MBED_ASSERT_WARN(can.write(txMsg) == true);
}

int main(void)
{
    pc.printf("Program Started\r\n\r\n");

    initCAN();

    servoController.setFunnelUp();
    elevatorController.runEndpointCalibration();
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