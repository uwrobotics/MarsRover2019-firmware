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
DigitalOut          led1(LED1);
Timer               timer;
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
 
int main(void)
{
    can.filter(RX_ID, CAN_MASK, CANStandard);

    while(1) {
        if(can.read(rxMsg)) {
            led1 = !led1;       // turn the LED on
            pc.printf("-------------------------------------\r\n");
            pc.printf("CAN message received\r\n");
            printCANMsg(rxMsg);
 
            // Filtering performed by software:
            if(rxMsg.id == RX_ID) {
                rxMsg >> counter;    // extract data from the received CAN message
                pc.printf("  counter = %d\r\n", counter);
            }
        }
    }
}
 
            