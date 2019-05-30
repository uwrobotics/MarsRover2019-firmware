#include "mbed.h"
#include "PinNames.h"
#include "rover_config.h"
#include "tgmath.h"
// emma blatt :)
DigitalOut led(LED1);
AnalogIn temp(PA_4);
Serial pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h

//temp reading specific
float temp_value_raw = 0;
float voltage_reading = 0;
float temp_val_k = 0;
float temp_val=0;
float R_temp = 0;
const float R0 = 100000;
const float T0 = 298.15;
const float B = 4700;
const float R1 = 150000;
const float R2 = 10000;
const float VDD = 3.3;

int main() {

    int i = 0;

    while (true) {
        temp_value_raw = temp.read();
        pc.printf("Raw Value: %f\r\n", temp_value_raw);
        voltage_reading = temp_value_raw * VDD;
        R_temp = 1/((1/(R2*(VDD/voltage_reading-1)))-(1/R1));
        temp_val_k = 1/(log(R_temp/R0)/B +(1/T0));
        temp_val = temp_val_k - 273.15;

        pc.printf("Temp: %f\r\n", temp_val);

        i++;
        wait(0.5);
        led = i % 2;
    }
}

