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
 * NOTE: When using an STM32F103C8T6 board uncomment line 22 and import the mbed-STM32F103C8T6 library 
 *
 * The same code is used for both mbed boards, but:
 *      For board #1 compile the example without any change.
 *      For board #2 comment out line 21 before compiling
 *
 * Once the binaries have been downloaded to the boards reset both boards at the same time.
 *
 */
 
//#define TARGET_STM32F103C8T6    1       // uncomment this line and import the mbed-STM32F103C8T6 library when using STM32F103C8T6 boards!
 
#define BOARD1                  1       // comment out this line when compiling for board #2
 
#if defined(TARGET_STM32F103C8T6)
#include "stm32f103c8t6.h"
#define LED_PIN PC_13
const int           OFF = 1;
const int           ON = 0;
#else
#define LED_PIN LED1
const int           OFF = 0;
const int           ON = 1;
#endif
#if defined(BOARD1)
const unsigned int  RX_ID = 0x100;
const unsigned int  TX_ID = 0x101;
#else
const unsigned int  RX_ID = 0x101;
const unsigned int  TX_ID = 0x100;
#endif
#include "mbed.h"
#include "CANMsg.h"
 
Serial              pc(USBTX, USBRX);
CAN                 can(PB_8, PB_9);  // CAN Rx pin name, CAN Tx pin name
CANMsg              rxMsg;
CANMsg              txMsg;
DigitalOut          led(LED_PIN);
Timer               timer;
uint8_t             counter = 0;
AnalogIn            analogIn(A0);
float               voltage;
 
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
 
/**
 * @brief   Main
 * @note
 * @param
 * @retval
 */
int main(void)
{
#if defined(TARGET_STM32F103C8T6)
    confSysClock();         //Configure system clock (72MHz HSE clock, 48MHz USB clock)
#endif
    pc.baud(9600);          // set Serial speed
    can.frequency(1000000); // set bit rate to 1Mbps
#if defined(BOARD1)
    led = ON;               // turn the LED on
    timer.start();          // start timer
    pc.printf("CAN_Hello board #1\r\n");
#else
    led = OFF;      // turn LED off
    pc.printf("CAN_Hello board #2\r\n");
#endif
    while(1) {
        if(timer.read_ms() >= 1000) {    // check for timeout
            timer.stop();                // stop timer
            timer.reset();               // reset timer
            counter++;                   // increment counter
            voltage = analogIn * 3.3f;   // read the small drifting voltage from analog input
            txMsg.clear();               // clear Tx message storage
            txMsg.id = TX_ID;            // set ID
            txMsg << counter << voltage; // append data (total data length must not exceed 8 bytes!)
            if(can.write(txMsg)) {       // transmit message
                led = OFF;               // turn the LED off
                pc.printf("-------------------------------------\r\n");
                pc.printf("CAN message sent\r\n");
                printMsg(txMsg);
                pc.printf("  counter = %d\r\n", counter);
                pc.printf("  voltage = %e V\r\n", voltage);
            }
            else
                pc.printf("Transmission error\r\n");
        }
 
        if(can.read(rxMsg)) {
            led = ON;       // turn the LED on
            pc.printf("-------------------------------------\r\n");
            pc.printf("CAN message received\r\n");
            printMsg(rxMsg);
 
            // Filtering performed by software:
            if(rxMsg.id == RX_ID) {
                rxMsg >> counter >> voltage;    // extract data from the received CAN message
                pc.printf("  counter = %d\r\n", counter);
                pc.printf("  voltage = %e V\r\n", voltage);
                timer.start();                  // transmission lag
            }
        }
    }
}
 
            