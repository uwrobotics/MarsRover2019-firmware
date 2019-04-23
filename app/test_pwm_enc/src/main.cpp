#include "mbed.h"
#include "PwmIn.h"

Serial pc(SERIAL_TX, SERIAL_RX);

DigitalOut led(LED1);
DigitalOut motorDirection(MOTOR1_DIR);

PwmOut pwm1(MOTOR3);
PwmIn a(ENC_A1, 120);

double avgEncPWMDuty = 0.0;
double prevAvgEncPWMDuty = 0.0;
double dutyVelocity = 0.0;
double angularVelocity = 0.0;

const float kUpdatePeriod = 0.04;

Timer timer;


int main() {

    motorDirection = 1;

    float elapsedTime = 0.0;

    int period_ms = 1; // Equivalent to 1kHz frequency
    float duty = 0.0f;

    // Specify PWM period
    pwm1.period_ms(period_ms);

    timer.start();

    while (true) {
        elapsedTime += kUpdatePeriod;
        led = !led;

        if (elapsedTime > 5.0) {
            // Set the duty cycle on the pins
            duty += 0.1f;
            pwm1.write(duty);
            pc.printf("pwm set to %.2f %%\r\n", pwm1.read() * 100);
            elapsedTime = 0.0;
        }

        if (duty > 0.3f)
        {
            duty = 0.0f;
        }

        avgEncPWMDuty = a.avgDutyCycle();
        dutyVelocity = (avgEncPWMDuty - prevAvgEncPWMDuty) / timer.read();
        timer.reset();
        prevAvgEncPWMDuty = avgEncPWMDuty;

        angularVelocity = 360.0 * dutyVelocity;

        pc.printf("Avg PW: %+f, \tAvg Prd: %+f, \tRaw Duty: %+f, \tAvg Duty: %+f, \tAvg Duty Velo: %+f, \tAng Veloc: %+f\r\n", a.avgPulseWidth(), a.avgPeriod(), a.dutyCycle(), avgEncPWMDuty, dutyVelocity, angularVelocity);

        wait(kUpdatePeriod);

    }
}
