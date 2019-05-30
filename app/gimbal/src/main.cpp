#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"
#include "PinNames.h"
#include "Servo.h"


// const ServoController::t_servoConfig servoConfig {
//     .funnelServoPin = SERVO_F,
//     .funnelUpPos = 0.7,
//     .funnelRestPos = 0.55,
//     .funnelDownPos = 0.3,

//     .probeServoPin = SERVO_P,
//     .probeUpPos = 0.5,
//     .probeDownPos = 0.5
// };

Serial             pc(SERIAL_TX, SERIAL_RX, ROVER_DEFAULT_BAUD_RATE);
CAN                can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg             rxMsg;

DigitalOut         ledErr(LED1);
DigitalOut         ledCAN(LED4);
Servo 				servo_yaw(SERVO_YAW);

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

enum gimbalCommand {
    setStepPitch,
	setStepYaw,
};

void initCAN() {
    can.filter(ROVER_GIMBAL_CANID, ROVER_CANID_FILTER_MASK, CANStandard);
}

void moveRight(void){
	servo_yaw = 0.2;
	wait(0.2);
	servo_yaw = 0.50;
	wait(1);
}

void moveLeft(void){
        servo_yaw = 0.8;
        wait(0.2);
        servo_yaw = 0.50;
        wait(1);
}

// bool handleStepPitch(CANMsg *p_newMsg){
// 	bool up = 0;
// 	*p_newMsg >> up;

//     if (up) {
//         // servoController.setGimbalUp();
// 		pitchPos += 0.05;
// 		servoPitch = pitchPos;
//     } else { // down
//         servoController.setGimbalDown();
//     }
//     return up;
// }

bool handleStepYaw(CANMsg *p_newMsg){
	bool right = 0;
	*p_newMsg >> right;

    if (right) {
        moveRight();
    } else { // left
        moveLeft();
    }
    return right;
}

void processCANMsg(CANMsg *p_newMsg) {
    if (p_newMsg->id == setStepYaw) {
		handleStepYaw(p_newMsg);
    } else {
		pc.printf("Unimplemented command\n");
	}
}


int main(void)
{
    pc.printf("Program Started\r\n\r\n");

    initCAN();

    while (1) {
        if (can.read(rxMsg)) {
            processCANMsg(&rxMsg);
            rxMsg.clear();
            ledCAN = !ledCAN;
        }
    }
}