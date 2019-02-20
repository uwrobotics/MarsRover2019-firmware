#include "mbed.h"
#include "PwmIn.h"

DigitalOut led(LED1);
Serial pc(SERIAL_TX, SERIAL_RX);

PwmIn a(ENC_A1);
PwmIn b(ENC_A2);
// PwmIn c(ENC_A3;

int main() {

    int i = 0;
    pc.baud(115200); 

    while (true) {

        led = i % 2;
        i++;
     
        pc.printf("a: dc = %f, pw = %f, period = %f\r\n",     a.dutyCycle(), a.pulseWidth(), a.period());
        pc.printf("b: dc = %f, pw = %f, period = %f\r\n",     b.dutyCycle(), b.pulseWidth(), b.period());
        // pc.printf("c: dc = %f, pw = %f, period = %f\r\n\r\n", c.dutycycle(), c.pulsewidth(), c.period());
        
        wait(0.25);

    }
}
