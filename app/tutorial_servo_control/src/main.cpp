#include "mbed.h"
#include "CANMsg.h"
#include "TutorialServo.h"

#define COMM_FREQ 10000
#define SERVO_SET_POS_ID 100
#define ROM_CANID 101

AnalogIn potVoltageIn(PA_0);
PwmOut servoPwmOut(PA_1);

// instance of servo object
TutorialServo tutorialServo(PA_1);

CAN can(PB_8, PB_9, COMM_FREQ);
Serial pc(USBTX, USBRX);
CANMsg rxMsg;
CANMsg txMsg;
uint8_t position;
uint16_t range;

// uncomment/comment desired mode
#define DO_CAN
// #define DO_POT

void processCANMsg(CANMsg *p_newMsg) {
    pc.printf("Got CAN msg with ID %X\r\n", p_newMsg->id);

    switch (p_newMsg->id) {
        case ROM_CANID:
            pc.printf("Received message for ROM, value: \r\n");
            for(int i = 0; i < p_newMsg->len; i++)
        		pc.printf(" 0x%.2X",p_newMsg->data[i]);

            *p_newMsg >> range;
        	// set ROM limit
        	tutorialServo.setRangeInDegrees(range); 

            // print out new ROM value
            pc.printf("New max range angle: %f \r\n", tutorialServo.getRangeInDegrees());
            break;

        case SERVO_SET_POS_ID:;
            pc.printf("Received message to set position, value: \r\n");
            for(int i = 0; i < p_newMsg->len; i++)
        		pc.printf(" 0x%.2X",p_newMsg->data[i]);

            *p_newMsg >> position;   
        	// set servo position
        	tutorialServo.setPositionInDegrees(position); 
            break;

        default:
            pc.printf("Recieved unimplemented command\r\n");
            break;
    }
}

// CAN code
int main() {
	pc.baud(9600);
    servoPwmOut.period(0.020);

    // infinite loop
    while(1) {

#ifdef DO_CAN
    	// receive can messages continually
    	if (can.read(rxMsg)) {
            processCANMsg(&rxMsg);
            rxMsg.clear();
        }
#endif

#ifdef DO_POT
    	float potVoltage = potVoltageIn.read();
    	servoPwmOut.pulsewidth(0.000625 + 0.002*potVoltage);
#endif

    }
}
