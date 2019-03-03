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

PID velocityPIDController(0.45, 0.7, 0.0, kUpdatePeriod);
Timer timer;

// Variables
float motorPWMDuty = 0.0;
float encoderPWMDuty = 0.0;
float prevEncoderPWMDuty = 0.0;
float angularVelocity = 0.0;

// Velocity to reach.

float angularVelocityGoal = 150;

// Initialize motor
void initializeMotor(void){
    motor.period_ms(1); // 1 ms period = 1 kHz frequency
    motor.write(0.0);
    motorDirection = 1;
}
 
// Setup velocity PID controller
void initializePidController(void){
    velocityPIDController.setInputLimits(0.0, 200.0);
    velocityPIDController.setOutputLimits(0.0, 0.5);
    velocityPIDController.setBias(0.0);
    velocityPIDController.setMode(PID_AUTO_MODE);
}
 
int main() {
 
    // Initialization
    initializeMotor();
    initializePidController();

    wait(1);

    // Set velocity set point.
    velocityPIDController.setSetPoint(angularVelocityGoal);

    timer.start();
 
    // Run for 3 seconds.
    while (true) {
        // Get the encoder PWM duty and calculate the angular velocity
        encoderPWMDuty = pwmEncoder.avgDutyCycle();
        angularVelocity = 360.0 * (encoderPWMDuty - prevEncoderPWMDuty) / timer.read(); // Degrees per second
        timer.reset();
        prevEncoderPWMDuty = encoderPWMDuty;

        // Update the PID controller
        velocityPIDController.setProcessValue(angularVelocity);
        motorPWMDuty = velocityPIDController.compute();
        motor.write(motorPWMDuty);

        pc.printf("%f, %f, %f\r\n", angularVelocity, angularVelocityGoal, motorPWMDuty);

        wait(kUpdatePeriod);
        led = !led;
    }
 
    // Stop motors
    motor.write(0.0);
    
}
