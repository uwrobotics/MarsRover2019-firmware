// Example using PwmIn interface library, sford
//  - Note: uses InterruptIn, so not available on p19/p20
 
#include "mbed.h"
#include "PwmIn.h"

PwmIn a(ENC_TT);
PwmIn b(ENC_SH);
PwmIn c(ENC_EL);
 
int main() {

    while(1) {
        printf("a: pw = %f, period = %f\n", a.pulsewidth(), a.period());
        printf("b: pw = %f, period = %f\n", b.pulsewidth(), b.period());
        printf("c: pw = %f, period = %f\n", c.pulsewidth(), c.period());
        wait(2);
    }
}