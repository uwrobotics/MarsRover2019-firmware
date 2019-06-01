#include "mbed.h"

DigitalIn lim_1a(LIM_1A);
DigitalIn lim_1b(LIM_1B);
DigitalIn lim_2a(LIM_2A);
DigitalIn lim_2b(LIM_2B);
DigitalIn lim_3a(LIM_3A);
DigitalIn lim_3b(LIM_3B);

Serial pc(SERIAL_TX, SERIAL_RX);

int main() {
    while(1) {
        pc.printf("LIM_1A: %d\r\n",     lim_1a.read());
        pc.printf("LIM_1B: %d\r\n\r\n", lim_1b.read());

        pc.printf("LIM_2A: %d\r\n",     lim_2a.read());
        pc.printf("LIM_2B: %d\r\n\r\n", lim_2b.read());

        pc.printf("LIM_3A: %d\r\n",     lim_3a.read());
        pc.printf("LIM_3B: %d\r\n\r\n", lim_3b.read());

        wait(0.5);
    }
}
