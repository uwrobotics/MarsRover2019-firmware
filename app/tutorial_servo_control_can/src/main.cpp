#include "mbed.h"
#include "CANMsg.h"
#include "servo_control.h"
#include "servo_control.cpp"

AnalogIn potVoltageIN(PA_0);
PwmOut servoPwmOut(PA_1);

CAN can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg txMsg;

const float ANGLERANGE = 270;
const float PULSEWIDTHRANGEMIN = 1;
const float PULSEWIDTHRANGEMAX = 2;

void initCAN()
{
    can.filter(RX_ID, ROVER_CANID_FILTER_MASK, CANStandard);
}

int main() {
    
    initCAN();

    // initialize servo parameters
    TutorialServo();
    TutorialServo.setAngleRangeInDegrees(ANGLERANGE);
    TutorialServo.setPulsewidthRangeInMs(PULSEWIDTHRANGEMIN, PULSEWIDTHRANGEMAX);

    while(1) {
        if(can.read(txMsg))
        {
            float servoPosition = 0;
            txMsg >> servoPosition;
            // not exactly sure if message reading was done correctly
            TutorialServo.setPositionInDegrees(servoPosition);
            txMsg.clear();
        }
    }
}
