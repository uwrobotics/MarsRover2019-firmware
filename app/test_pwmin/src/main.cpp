#include "mbed.h"
#include "PwmIn.h"

DigitalOut led(LED1);
Serial pc(SERIAL_TX, SERIAL_RX);

PwmIn a(ENC_TT);
PwmIn b(ENC_SH);
PwmIn c(ENC_EL);

int main() {

    int i = 0;
    pc.baud(9600); 

    while (true) {

        led = i % 2;
        i++;
     
        pc.printf("a: pw = %f, period = %f\r\n",     a.pulsewidth(), a.period());
        pc.printf("b: pw = %f, period = %f\r\n",     b.pulsewidth(), b.period());
        pc.printf("c: pw = %f, period = %f\r\n\r\n", c.pulsewidth(), c.period());
        
        wait(1);

    }
}
