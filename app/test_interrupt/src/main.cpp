
#include "mbed.h"
 
InterruptIn button(ENC_A2);
DigitalOut led(LED1);
DigitalOut flash(LED4);
 
void ledOn() {
    led = 1;
}

void ledOff() {
    led = 0;
}
 
int main() {
    button.rise(&ledOn);  // attach the address of the flip function to the rising edge
    button.fall(&ledOff);
    while(1) {           // wait around, interrupts will interrupt this!
        flash = !flash;
        wait(0.25);
    }
}