#include "mbed.h"

DigitalOut led(LED1);
Serial pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h

int main() {

    int i = 0;

    while (true) {
        pc.printf("Hello World! %d\r\n", i);
        i++;
        wait(0.5);
        led = i % 2;
    }
}

