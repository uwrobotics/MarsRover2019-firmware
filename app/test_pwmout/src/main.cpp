#include "mbed.h"
 
PwmOut led(LED1);
PwmOut pwm1(PWM1);
 
int main() {

	float period = 4.0f;
    float duty = 0.5f;

    // Specify PWM period
    led.period(period);  // 4 second period
    pwm1.period(period);

    // Slowly increase the duty cycle. Once it is 100%, set back to 0%.
    while(1) {
    	if (duty > 1.0f)
    	{
    		duty = 0.0f;
    	}

    	// Set the duty cycle on the pins
    	led.write(duty);
    	pwm1.write(duty);

    	// Increment the duty cycle
    	// duty += 0.1f;

    	wait(0.5);
    }
}