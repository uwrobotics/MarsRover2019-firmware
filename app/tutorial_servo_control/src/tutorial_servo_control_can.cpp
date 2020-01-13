#include "CANMsg.h"
#include "rover_config.h"
#include "mbed.h"
#include "TutorialServo.h"

int main(){
    TutorialServo myServo(PA_1);
    CAN           can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
    CANMsg        rxMsg;

    while(1){
        //assume the percentage looks like 0.5
        if (can.read(rxMsg)){
            float percentage = rxMsg.data[0];
            float angle = myServo.getServoRangeInDegrees() * percentage;
            myServo.setPositionInDegrees(angle);
        }
    }
}