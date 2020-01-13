#include "mbed.h"

AnalogIn potVoltageIn(PA_0);
PwmOut servoPwmout(PA_1);

int main() {

    servoPwmout.period(0.02);
    float output_voltage = 0.0f;
    float pwm = 0.0f;
    while(1) {
        output_voltage = potVoltageIn.read();
        pwm = ((output_voltage/3.3)+1)/1000.0;
        servoPwmout.pulsewidth(pwm);        
    }
}