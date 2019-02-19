
#include "QEI.h"
 
Serial pc(SERIAL_TX, SERIAL_RX);
//Use X4 encoding.
//QEI wheel(p29, p30, NC, 624, QEI::X4_ENCODING);
//Use X2 encoding by default.
QEI wheel (PA_7, PC_4, PC_5, 48, QEI::X4_ENCODING);

// pin mappings when using science board EC1:
// ENC_CENT_1 -> PA7 -> LQFP64 -> 23
// ENC_CENT_2 -> PC4 -> LQFP64 -> 24
// END_CENT_INDEX -> PC5 -> LQFP64 -> 25
 
int main() {
    pc.baud(9600);
 
    while(1){
        wait(0.1);
        pc.printf("Pulses is: %i\n\r", wheel.getPulses());
    }
 
}
