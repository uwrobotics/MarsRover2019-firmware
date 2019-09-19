#include "mbed.h"
#include "CANMsg.h"

#define COMM_FREQ 9600
#define ROM_CANID 69
#define SERVO_SET_POS_ID 21

AnalogIn potVoltageIn(PA_0);
PwmOut servoPwmOut(PA_1);

// instance of servo object
TutorialServo TutorialServo(PA_0);
CAN can(CAN_RX, CAN_TX, COMM_FREQ);
CANMsg rxMsg;
CANMsg txMsg;

void processCANMsg(CANMsg *p_newMsg) {
    pc.printf("Got CAN msg with ID %X\r\n", p_newMsg->id);

    switch (p_newMsg->id) {

        case ROM_CANID:
            pc.printf("Received message for ROM, value: \r\n");
            for(int i = 0; i < p_newMsg->len; i++)
        		pc.printf(" 0x%.2X",p_newMsg->data[i]);

        	// set ROM limit
        	TutorialServo.setRangeInDegrees(21.0); // replace with actual float value
            break;

        case SERVO_SET_POS_ID:
            pc.printf("Received message to set position, value: \r\n");
            for(int i = 0; i < p_newMsg->len; i++)
        		pc.printf(" 0x%.2X",p_newMsg->data[i]);

        	// set servo position
        	TutorialServo.setPositionInDegrees(69.34); // replace with actual float value
            break;

        default:
            pc.printf("Recieved unimplemented command\r\n");
            break;
    }
}

// pot code
int main() {
    servoPwmOut.period(0.020);

    while(1) {
    	float potVoltage = potVoltageIn.read();
    	servoPwmOut.pulsewidth(0.001 + 0.001*potVoltage/3.3);
    }
}

// // CAN code
// int main() {

//     while(1) {
//     	// receive can messages with range of motion
//     	if (can.read(rxMsg)) {
//             processCANMsg(&rxMsg);
//             rxMsg.clear();
//         }
//     }
// }
