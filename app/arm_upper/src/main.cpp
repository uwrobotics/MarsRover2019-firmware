#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"
#include "PwmIn.h"
#include "PID.h"
#include "Motor.h"
#include "ArmJointController.h"
#include "ArmWristController.h"
#include "ArmClawController.h"

const ArmWristController::t_armWristConfig wristConfig = {
        .leftJointConfig = {
                .motor = {
                        .pwmPin = MOTOR1,
                        .dirPin = MOTOR1_DIR,
                        .inverted = true,
                        .freqInHz = MOTOR_DEFAULT_FREQUENCY_HZ,
                        .limit = 1.0
                },

                .encoder = {
                        .pwmPin = ENC_A1,
                        .zeroAngleDutyCycle = 0.497f,
                        .minAngleDegrees = -170.0f,
                        .maxAngleDegrees = 170.0f,
                        .inverted = true
                },

                .limSwitchMinPin = LIM_1A,
                .limSwitchMaxPin = LIM_1B,

                .velocityPID = {
                        .P    = 0.15f,
                        .I    = 0.45f,
                        .D    = 0.0f,
                        .bias = 0.0f,
                        .interval = 0.05f
                },

                .positionPID = {
                        .P    = 5.5f,
                        .I    = 0.0f,
                        .D    = 0.0f,
                        .bias = 0.0f,
                        .interval = 0.05f
                },

                .minInputVelocityDegPerSec = -20.0f,
                .maxInputVelocityDegPerSec = 20.0f,
                .minOutputMotorDutyCycle = -1.0f,
                .maxOutputMotorDutyCycle = 1.0f
        },

        .rightJointConfig = {
                .motor = {
                        .pwmPin = MOTOR2,
                        .dirPin = MOTOR2_DIR,
                        .inverted = false,
                        .freqInHz = MOTOR_DEFAULT_FREQUENCY_HZ,
                        .limit = 1.0
                },

                .encoder = {
                        .pwmPin = ENC_A2,
                        .zeroAngleDutyCycle = 0.498f,
                        .minAngleDegrees = -170.0f,
                        .maxAngleDegrees = 170.0f,
                        .inverted = false
                },

                .limSwitchMinPin = LIM_2A,
                .limSwitchMaxPin = LIM_2B,

                .velocityPID = {
                        .P    = 0.3f,
                        .I    = 0.2f,
                        .D    = 0.0f,
                        .bias = 0.0f,
                        .interval = 0.05f
                },

                .positionPID = {
                        .P    = 15.7f,
                        .I    = 0.0f,
                        .D    = 0.0f,
                        .bias = 0.0f,
                        .interval = 0.05f
                },

                .minInputVelocityDegPerSec = -20.0f,
                .maxInputVelocityDegPerSec = 20.0f,
                .minOutputMotorDutyCycle = -1.0f,
                .maxOutputMotorDutyCycle = 1.0f
        },

        .leftToRightMotorBias = 0.0f
};

const ArmClawController::t_clawConfig clawConfig = {
        .motor = {
                .pwmPin   = MOTOR3,
                .dirPin   = MOTOR3_DIR,
                .inverted = false,
                .freqInHz = MOTOR_DEFAULT_FREQUENCY_HZ,
                .limit = 1.0
        },

        .encoder = {
                .channelAPin = ENCR1_CH1,
                .channelBPin = ENCR1_CH2,
                .indexPin    = ENCR1_INDEX,

                .pulsesPerRevolution = 360,
                .encoding = QEI::X4_ENCODING,
                .inverted = false
        },

        .limitSwitchPin = LIM_3A,

        .calibrationDutyCycle = 0.2f,
        .calibrationTimeoutSeconds = 7.0f,

        .positionPID = {
                .P = 5.0f,
                .I = 0.0f,
                .D = 0.0f,

                .bias     = 0.0f,
                .interval = 0.05f
        },

        .minInputSeparationDistanceCm = 0.0f,
        .maxInputSeparationDistanceCm = 120.0f,

        .minOutputMotorDutyCycle = -1.0f,
        .maxOutputMotorDutyCycle = 1.0f

};


Serial             pc(SERIAL_TX, SERIAL_RX, ROVER_DEFAULT_BAUD_RATE);
CAN                can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg             rxMsg;

DigitalOut         ledErr(LED1);
DigitalOut         ledCAN(LED4);

ArmWristController wristController(wristConfig, ArmJointController::velocityPID);
ArmClawController  clawController(clawConfig, ArmClawController::positionPID);

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

enum armCommand {

    setWristControlMode = ROVER_ARM_UPPER_CANID,
    setWristPitchMotion,
    setWristRollMotion,
    setClawControlMode,
    setClawMotion,

    firstCommand = setWristControlMode,
    lastCommand  = setClawMotion

};

enum jetsonFeedback {
    wristPitchDegrees = ROVER_JETSON_START_CANID_MSG_ARM_UPPER,
    wristRollDegrees,
    clawSeparationDistanceCm,
};

void initCAN() {
    can.filter(ROVER_ARM_UPPER_CANID, ROVER_CANID_FILTER_MASK, CANStandard);
}

ArmJointController::t_jointControlMode handleSetWristControlMode(CANMsg *p_newMsg) {
    ArmJointController::t_jointControlMode controlMode;
    *p_newMsg >> controlMode;

    MBED_WARN_ON_ERROR(wristController.setControlMode(controlMode));

    return controlMode;
}

ArmClawController::t_clawControlMode handleSetClawControlMode(CANMsg *p_newMsg) {
    ArmClawController::t_clawControlMode controlMode;
    *p_newMsg >> controlMode;

    MBED_WARN_ON_ERROR(clawController.setControlMode(controlMode));

    return controlMode;
}

float handleSetWristPitchMotion(CANMsg *p_newMsg) {
    float motionData = 0;
    *p_newMsg >> motionData;

    ArmJointController::t_jointControlMode controlMode = wristController.getControlMode();

    switch (controlMode) {

        case ArmJointController::motorDutyCycle:
            MBED_WARN_ON_ERROR(wristController.setPitchDutyCycle(motionData));
            break;

        case ArmJointController::velocityPID:
            MBED_WARN_ON_ERROR(wristController.setPitchVelocityDegreesPerSec(motionData));
            break;

        case ArmJointController::positionPID:
            MBED_WARN_ON_ERROR(wristController.setPitchAngleDegrees(motionData));
            break;
    }

    return motionData;
}

float handleSetWristRollMotion(CANMsg *p_newMsg) {
    float motionData = 0;
    *p_newMsg >> motionData;

    ArmJointController::t_jointControlMode controlMode = wristController.getControlMode();

    switch (controlMode) {

        case ArmJointController::motorDutyCycle:
            MBED_WARN_ON_ERROR(wristController.setRollDutyCycle(motionData));
            break;

        case ArmJointController::velocityPID:
            MBED_WARN_ON_ERROR(wristController.setRollVelocityDegreesPerSec(motionData));
            break;

        case ArmJointController::positionPID:
            MBED_WARN_ON_ERROR(wristController.setRollAngleDegrees(motionData));
            break;
    }

    return motionData;
}

float handleSetClawMotion(CANMsg *p_newMsg) {
    float motionData = 0;
    *p_newMsg >> motionData;

    ArmClawController::t_clawControlMode controlMode = clawController.getControlMode();

    switch (controlMode) {

        case ArmClawController::motorDutyCycle:
            MBED_WARN_ON_ERROR(clawController.setMotorDutyCycle(motionData));
            break;

        case ArmClawController::positionPID:
            MBED_WARN_ON_ERROR(clawController.setSeparationDistanceCm(motionData));
            break;
    }

    return motionData;
}

void processCANMsg(CANMsg *p_newMsg) {
    switch (p_newMsg->id) {

        case setWristControlMode:
            handleSetWristControlMode(p_newMsg);
            break;

        case setWristPitchMotion:
            handleSetWristPitchMotion(p_newMsg);
            break;

        case setWristRollMotion:
            handleSetWristRollMotion(p_newMsg);
            break;

        case setClawControlMode:
            handleSetClawControlMode(p_newMsg);
            break;

        case setClawMotion:
            handleSetClawMotion(p_newMsg);
            break;

        default:
            pc.printf("Recieved unimplemented command\r\n");
            break;
    }
}

void sendJetsonInfo() {

    CANMsg txMsg(0);

    txMsg.clear();
    txMsg.id = wristPitchDegrees;
    txMsg << wristController.getPitchAngleDegrees();
    MBED_ASSERT(can.write(txMsg) == true);

    txMsg.clear();
    txMsg.id = wristRollDegrees;
    txMsg << wristController.getRollAngleDegrees();
    MBED_ASSERT(can.write(txMsg) == true);

    txMsg.clear();
    txMsg.id = clawSeparationDistanceCm;
    txMsg << clawController.getSeparationDistanceCm();
    MBED_ASSERT(can.write(txMsg) == true);

}

int main(void)
{
    pc.printf("Program Started\r\n\r\n");

    initCAN();
    canSendTimer.start();

    MBED_WARN_ON_ERROR(clawController.runEndpointCalibration());

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

        wristController.update();
        clawController.update();

    }
}

