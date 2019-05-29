#include "mbed.h"
#include "PinNames.h"
#include "rover_config.h"

DigitalOut led(LED1);
AnalogIn            battery(V_MONITOR);
Serial pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h

// Voltage monitoring specific
const float compression_factor = 0.110629;
const int cell_num = 6;
const float full_cell = 4.20;
const float low_cell = 3.75; // TODO verify this value
const float full_bat = full_cell * cell_num;
const float low_bat = low_cell * cell_num;
const float board_voltage = 3.3;
float bat_value = 0;
float bat_value_raw = 0;
#define V_INDEX 3

int main() {

    int i = 0;

    while (true) {
                // Reading of battery voltage
        /* Battery voltage conversion 
        * Voltage divider of 82k and 10.2k provide a compression factor of 0.110629
		* 10.19k, 81.8k
        * Does CAN Msg need to be sent out?
        * 400 mV when 5V is on
        * 0.16 reading
        */
       bat_value_raw = battery.read();
       bat_value = bat_value_raw * board_voltage/ compression_factor;
       pc.printf("Battery Level: %f\r\n", bat_value);
       if (bat_value < low_cell){
           // CANMsg txMsg(TX_ID + V_INDEX);
           pc.printf("REPLACE BATTERY, Battery Level LOW!!! \r\n");
       }

        pc.printf("Hello World! %d\r\n", i);
        i++;
        wait(0.5);
        led = i % 2;
    }
}

