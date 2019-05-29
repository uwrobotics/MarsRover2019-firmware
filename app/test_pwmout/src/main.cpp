#include "mbed.h"

DigitalOut led(LED1);
PwmOut 	   pwm1(MOTOR_C);
Serial pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h
 
int main() {

	int period_ms = 1; // Equivalent to 1kHz frequency
    float duty = 0.0f;

    // Specify PWM period
    pwm1.period_ms(period_ms);

    // Slowly increase the duty cycle. Once it is 100%, set back to 0%.
    while(1) {
    	if (duty > 1.001f)
    	{
    		duty = 0.0f;
    	}

    	// Set the duty cycle on the pins
    	pwm1.write(duty);
    	pc.printf("pwm set to %.2f %%\n", pwm1.read() * 100);

    	// Increment the duty cycle
    	duty += 0.1f;

    	led = !led;

    	wait(1);
    }
}