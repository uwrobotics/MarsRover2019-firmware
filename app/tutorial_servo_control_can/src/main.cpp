#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"
#include "TutorialServo.h"

int main(){
    TutorialServo myServo(PA_1);
    CAN           can(PB_8, PB_9, ROVER_CANBUS_FREQUENCY);
    CANMsg        rxMsg;

    while(1){
        //assume the percentage looks like 0.5
        if (can.read(rxMsg)){
            float percentage = 0;
            rxMsg >> percentage;
            float angle = myServo.getServoRangeInDegrees() * percentage;
            myServo.setPositionInDegrees(angle);
        }
    }
}