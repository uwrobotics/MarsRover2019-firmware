#include "mbed.h"
#include "PwmIn.h"

DigitalOut led(LED1);
DigitalOut motorDirection(MOTOR1_DIR);

Serial pc(SERIAL_TX, SERIAL_RX);

PwmIn a(ENC_A1, 20);
// PwmIn b(ENC_A2);
// PwmIn c(ENC_A3;


PwmOut     pwm1(MOTOR1);

double encoderPWMDuty = 0.0;
double prevEncoderPWMDuty = 0.0;
double dutyVelocity = 0.0;
double angularVelocity = 0.0;

const float kUpdatePeriod = 0.05;


int main() {

    motorDirection = 1;

    int i = 0;
    pc.baud(115200); 

    int period_ms = 1; // Equivalent to 1kHz frequency
    float duty = 0.0f;

    // Specify PWM period
    pwm1.period_ms(period_ms);

    while (true) {
        if (duty > 0.3f)
        {
            duty = 0.0f;
        }

        if (i % 40 == 0) {
            // Set the duty cycle on the pins
            pwm1.write(duty);
            pc.printf("pwm set to %.2f %%\r\n", pwm1.read() * 100);
            duty += 0.1f;
        }

        led = i % 2;
        i++;

        encoderPWMDuty = a.avgDutyCycle();
        dutyVelocity = (encoderPWMDuty - prevEncoderPWMDuty) / kUpdatePeriod;
        angularVelocity = 360.0f * dutyVelocity;
        prevEncoderPWMDuty = encoderPWMDuty;

        // pc.printf("Duty: %f, \tDuty Velocity: %f, \tAngular Velocity: %f\r\n", encoderPWMDuty, dutyVelocity, angularVelocity);
        pc.printf("a: dc = %f, pw = %f, period = %f\r\n",     a.avgDutyCycle(), a.avgPulseWidth(), a.avgPeriod()); 

        wait(kUpdatePeriod);

    }
}
