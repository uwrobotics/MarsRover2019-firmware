
#include "QEI.h"
#include "Motor.h"
 
Serial pc(SERIAL_TX, SERIAL_RX);
//Use X4 encoding.
//QEI wheel(p29, p30, NC, 624, QEI::X4_ENCODING);
//Use X2 encoding by default.

// configured for use with science board centrifuge encoder
//QEI wheel (ENC_C_CH1, ENC_C_CH2, ENC_C_INDEX, 48, QEI::X4_ENCODING);

// configured for use with arm claw encoder
QEI wheel(ENC_E_CH1, ENC_E_CH2, ENC_E_INDEX, 211, QEI::X4_ENCODING);

Motor motor(MOTOR_E, MOTOR_E_DIR);

// pin mappings when using science board EC1:
// ENC_CENT_1 -> PA7 -> LQFP64 -> 23
// ENC_CENT_2 -> PC4 -> LQFP64 -> 24
// END_CENT_INDEX -> PC5 -> LQFP64 -> 25
 
int main() {

    motor.setDutyCycle(0.2);

    while(1){
        wait(0.1);
        pc.printf("Pulses is: %i\n\r", wheel.getPulses());
    }
 
}
