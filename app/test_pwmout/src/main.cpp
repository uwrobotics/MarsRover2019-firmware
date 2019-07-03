#include "mbed.h"
#include "Motor.h"

DigitalOut led(LED1);
Motor 	   pwm1(MOTOR3, MOTOR3_DIR);
Serial pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h
 
int main() {



	int period_ms = 1; // Equivalent to 1kHz frequency
    float duty = 0.0f;

    // Specify PWM period
//    pwm1.period_ms(period_ms);

    // Slowly increase the duty cycle. Once it is 100%, set back to 0%.
    while(1) {
    	if (duty > 1.0f)
    	{
    		duty = 0.0f;
    	}

    	// Set the duty cycle on the pins
    	pwm1.setDutyCycle(duty);
    	pc.printf("pwm set to %.2f %%\r\n", pwm1.getDutyCycle());

    	// Increment the duty cycle
    	duty += 0.1f;

    	led = !led;

    	wait(1);
    }
}