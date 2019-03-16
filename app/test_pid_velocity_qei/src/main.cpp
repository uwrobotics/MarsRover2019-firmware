#include "mbed.h"
#include "PID.h"
#include "PwmIn.h"
#include "QEI.h"

// Constants
const float kUpdatePeriod = 0.01;
const float kP = 1.0;
const float kI = 0.0;
const float kD = 0.0;

PwmOut motor(PC_7); // PWM_CENT
DigitalOut motorDirection(PC_8);  // DIR_CENT
QEI quadEncoder(PA_7, PC_4, PC_5, 2048);

DigitalOut led(LED1);

PID velocityPIDController(kP, kI, kD, kUpdatePeriod);
Timer endTimer;

Serial pc(SERIAL_TX, SERIAL_RX);

// Variables
float motorPWMDuty = 1.0;
int encoderPosition = 0;
int prevEncoderPosition = 0;
float angularVelocity = 0;

// Velocity to reach.
int goal = 3000;    // TODO tune

// Initialize motor
void initializeMotor(void){
    motor.period_ms(1); // 1 ms period = 1 kHz frequency
    motor.write(0.0f);
    motorDirection = 0;
}

// Setup velocity PID controller
void initializePidController(void){
    velocityPIDController.setInputLimits(0.0, 10000);     // TODO tune
    velocityPIDController.setOutputLimits(0.0, 1.0);
    velocityPIDController.setBias(1.0);
    velocityPIDController.setMode(PID_AUTO_MODE);
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
        // Get the encoder PWM duty and calculate the angular velocity
        encoderPosition = quadEncoder.getPulses();
        angularVelocity = 360.0f * (encoderPosition - prevEncoderPosition) / kUpdatePeriod; // Degrees per second
        prevEncoderPosition = encoderPosition;

        // Update the PID controller
        velocityPIDController.setProcessValue(angularVelocity);
        motorPWMDuty = velocityPIDController.compute();
        motor.write(motorPWMDuty);

        pc.printf("%f,%f\n", angularVelocity, goal);

        wait(kUpdatePeriod);
        led = !led;
    }

    // Stop motors
    motor.write(0.0);

}
