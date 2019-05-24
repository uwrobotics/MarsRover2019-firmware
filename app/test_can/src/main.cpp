/*
 * An example showing how to use the mbed CAN API:
 *
 * Two affordable (about $2 on ebay) STM32F103C8T6 boards (20kB SRAM, 64kB Flash),
 * (see [https://developer.mbed.org/users/hudakz/code/STM32F103C8T6_Hello/] for more details)
 * are connected to the same CAN bus via transceivers (MCP2551 or TJA1040, or etc.).
 * CAN transceivers are not part of NUCLEO boards, therefore must be added by you.
 * Remember also that CAN bus (even a short one) must be terminated with 120 Ohm resitors at both ends.
 *
 * For more details see the wiki page <https://developer.mbed.org/users/hudakz/code/CAN_Hello/>
 *
 * The same code is used for both mbed boards, but:
 *      For board #1 compile the example without any change.
 *      For board #2 comment out line 21 before compiling
 *
 * Once the binaries have been downloaded to the boards reset both boards at the same time.
 *
 */
  
//#define BOARD1 1 // comment out this line when compiling for board #2

#if defined(BOARD1)
const unsigned int  RX_ID = 0x100; 
const unsigned int  TX_ID = 0x101; 
#else
const unsigned int  RX_ID = 0x101;
const unsigned int  TX_ID = 0x100;
#endif

#include "mbed.h"
#include "CANMsg.h"

Serial              pc(SERIAL_TX, SERIAL_RX);
CAN                 can(CAN_RX, CAN_TX);
CANMsg              rxMsg;
CANMsg              txMsg;
DigitalOut          led(LED1);
Timer               timer;
uint8_t             counter = 0;
 
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
 
int main(void)
{
    can.frequency(500000); // set bit rate to 50kbps
#if defined(BOARD1)
    led = 1;                // turn the LED on
    timer.start();          // start timer
    pc.printf("CAN_Hello board #1\r\n");
#else
    led = 0;                // turn LED off
    pc.printf("CAN_Hello board #2\r\n");
#endif
    while(1) {
        if(timer.read_ms() >= 1000) {    // check for timeout
            timer.stop();                // stop timer
            timer.reset();               // reset timer
            counter++;                   // increment counter
            txMsg.clear();               // clear Tx message storage
            txMsg.id = TX_ID;            // set ID
            txMsg << counter;            // append data (total data length must not exceed 8 bytes!)
            if(can.write(txMsg)) {       // transmit message
                led = 0;                 // turn the LED off
                pc.printf("-------------------------------------\r\n");
                pc.printf("CAN message sent\r\n");
                printMsg(txMsg);
                pc.printf("  counter = %d\r\n", counter);
            }
            else
                pc.printf("Transmission error\r\n");
        }
 
        if(can.read(rxMsg)) {
            led = 1;       // turn the LED on
            pc.printf("-------------------------------------\r\n");
            pc.printf("CAN message received\r\n");
            printMsg(rxMsg);
 
            // Filtering performed by software:
            if(rxMsg.id == RX_ID) {
                rxMsg >> counter;    // extract data from the received CAN message
                pc.printf("  counter = %d\r\n", counter);
                timer.start();                  // transmission lag
            }
        }
    }
}
 
            