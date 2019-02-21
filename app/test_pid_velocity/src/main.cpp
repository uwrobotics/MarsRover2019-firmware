#include "mbed.h"
#include "PID.h"
#include "PwmIn.h"
 
// Constants
const float kRATE = 0.01;
const float kP    = 1.0;
const float KI    = 0.0;
const float KD    = 0.0;

PwmOut motor(MOTOR1);
DigitalOut motorDirection(MOTOR1_DIR);
PwmInPwmIn pwmEncoder(ENC_A1);

PID velocityPIDController(kP, kI, kD, kRATE);
Timer endTimer;

// Variables
volatile float motorPWMDuty = 1.0;
volatile float angularVelocity = 0.0;

// Velocity to reach.
int goal = 3000;

// Initialize motor
void initializeMotor(void){
    motor.period_ms(1);
    motor.wirte(0.0f);
    motorDirection = 0;
}
 
// Setup velocity PID controller
void initializePidController(void){
    velocityPIDController.setInputLimits(0.0, 1.0);
    velocityPIDController.setOutputLimits(0.0, 1.0);
    velocityPIDController.setBias(1.0);
    velocityPIDController.setMode(AUTO_MODE);
}
 
int main() {

    endTimer.start();
 
    // Initialization
    initializeMotor();
    initializePidController();
 
    // Set velocity set point.
    velocityPIDController.setSetPoint(goal);
 
    // Run for 3 seconds.
    while (endTimer.read() < 3.0) {
        velocityPIDController.setProcessValue(angularVelocity);
        motorPWMDuty = velocityPIDController.compute();
        motor.write(motorPWMDuty);
        pc.printf(fp, "%f,%f\n", angularVelocity, goal);
        wait(RATE);
    }
 
    // Stop motors
    motor.write(0.0);
    
}