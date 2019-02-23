#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"

const unsigned int  RX_ID = ROVER_ARMO_CANID; 
const unsigned int  TX_ID = ROVER_JETSON_CANID; 
const unsigned int  CAN_MASK = ROVER_CANID_FILTER_MASK;

Serial              pc(SERIAL_TX, SERIAL_RX, ROVER_DEFAULT_BAUD_RATE);
CAN                 can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg              rxMsg;
CANMsg              txMsg;
DigitalOut          ledErr(LED1);
DigitalOut          ledCAN(LED4);
uint8_t             counter = 0;
 
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
 
    configureVelocityPIDControl = RX_ID,
    setVelocityJoint1,
    setVelocityJoint2,
    setVelocityJoint3,
    configurePositionPIDControl,
    setPositionJoint1,
    setPositionJoint2,
    setPOsitionJoint3,
    configureDirectMotorControl,
    setDutyMotor1,
    setDutyMotor2,
    setDutyMotor3,

    firstCommand = configureVelocityPIDControl,
    lastCommand  = setDutyMotor3

};

void initCAN() {
    can.filter(ROVER_ARMO_CANID, ROVER_CANID_FILTER_MASK, CANStandard);

    // for (int canHandle = firstCommand; canHandle <= lastCommand; canHandle++) {
    //     can.filter(RX_ID + canHandle, 0xFFF, CANStandard, canHandle);
    // }
}

void proccessCANMsg(CANMsg msg) {
    switch (rxMsg.id) {
        case configureDirectMotorControl: 
            pc.printf("Recieved command configureDirectMotorControl\r\n");
            break;
        case setDutyMotor1:
            pc.printf("Recieved command setDutyMotor1\r\n");
            break;
        default:
            pc.printf("Recieved unimplemented command\r\n");
            break;
    }
}
 
int main(void)
{
    initCAN();

    while (1) {

        if (can.read(rxMsg)) {
            proccessCANMsg(rxMsg);
            ledCAN = !ledCAN;
        }

    }
}
 
            