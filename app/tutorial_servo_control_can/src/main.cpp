#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"
#include "TutorialServo.h"

#define BOARD1
#if defined(BOARD1)
const unsigned int  RX_ID = 0x100; 
#else
const unsigned int  RX_ID = 0x101;
#endif

#define ROVERBOARD_COMMON_PINMAP

Serial  pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h
CAN     can(CAN_RX, CAN_TX);
CANMsg  rxMsg;
uint8_t pos = 0;

/**
 * @brief   Prints CAN msg to PC's serial terminal
 * @note
 * @param   CANMessage to print
 * @retval
 */
void printMsg(CANMessage& msg) {
    pc.printf("  ID      = 0x%.3x\r\n", msg.id);
    pc.printf("  Type    = %d\r\n", msg.type);
    pc.printf("  Format  = %d\r\n", msg.format);
    pc.printf("  Length  = %d\r\n", msg.len);
    pc.printf("  Data    =");
    for(int i = 0; i < msg.len; i++)
        pc.printf(" 0x%.2X", msg.data[i]);
    pc.printf("\r\n");
}

int main()
{
    TutorialServo servo_1(PA_1, 180);

    while (1)
    {
        if(can.read(rxMsg)) {
            pc.printf("-------------------------------------\r\n");
            pc.printf("CAN message received\r\n");
            printMsg(rxMsg);
 
            // Filtering performed by software:
            if(rxMsg.id == RX_ID) {
                rxMsg >> pos;    // extract data from the received CAN message
                pc.printf("pos = %d\r\n", pos);

                servo_1.setPositionInDegrees(pos);
            }
        }
    } 
}
