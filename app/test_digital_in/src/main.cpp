#include "mbed.h"

DigitalOut ledA(LED1);
DigitalIn  digIn(LIM_3A);
Serial pc(SERIAL_TX, SERIAL_RX);

int main() {
    int count = 0;

    while(1) {
        pc.printf("Dig in: %d", digIn.read());
        ledA = digIn.read();
        wait(0.5);
    }
}
