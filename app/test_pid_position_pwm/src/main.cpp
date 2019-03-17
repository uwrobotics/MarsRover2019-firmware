#include "mbed.h"
#include "PID.h"
#include "PwmIn.h"
 
// Constants
const float kUpdatePeriod = 0.05;

Serial pc(SERIAL_TX, SERIAL_RX);

PwmOut motor(MOTOR1);
DigitalOut motorDirection(MOTOR1_DIR);
PwmIn pwmEncoder(ENC_A1, 12);

DigitalOut led(LED1);

PID velocityPIDController(3.4, 0.6, 0.0, kUpdatePeriod);

// Variables
float motorPWMDuty = 0.0;
float encoderPWMDuty = 0.0;

// Velocity to reach.
float angleGoal = 0.9;

// Initialize motor
void initializeMotor(void){
    motor.period_ms(1); // 1 ms period = 1 kHz frequency
    motor.write(0.0);
    motorDirection = 1;
}
 
// Setup velocity PID controller
void initializePidController(void){
    velocityPIDController.setInputLimits(0.0, 1.0);
    velocityPIDController.setOutputLimits(-0.3, 0.3);
    velocityPIDController.setBias(0.0);
    velocityPIDController.setMode(PID_AUTO_MODE);
}

void setMotor(float speed) {
    if (speed >= 0) {
        motorDirection = 1;
    }
    else {
        motorDirection = 0;
        speed = -speed;
    }

    motor.write(speed);
}
 
int main() {
 
    // Initialization
    initializeMotor();
    initializePidController();

    wait(2);

    // Set velocity set point.
    velocityPIDController.setSetPoint(angleGoal);
 
    // Run for 3 seconds.
    while (true) {
        // Get the encoder PWM duty and calculate the angular velocity
        encoderPWMDuty = pwmEncoder.avgDutyCycle();

        // Update the PID controller
        velocityPIDController.setProcessValue(encoderPWMDuty);
        motorPWMDuty = velocityPIDController.compute();
        setMotor(motorPWMDuty);

        pc.printf("%f, %f, %f\r\n", encoderPWMDuty, angleGoal, motorPWMDuty);

        wait(kUpdatePeriod);
        led = !led;
    }
 
    // Stop motors
    motor.write(0.0);
    
}
