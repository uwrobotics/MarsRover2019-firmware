#include "mbed.h"
#include "PID.h"
#include "PwmIn.h"
#include "Motor.h"

// Constants
const float kUpdatePeriod = 0.05;

Serial pc(SERIAL_TX, SERIAL_RX);

Motor motor(MOTOR1, MOTOR1_DIR, true);
PwmIn pwmEncoder(ENC_A1, 50);

DigitalOut led(LED1);

PID velocityPIDController(0.15, 0.45, 0.0, kUpdatePeriod);
Timer timer;

// Variables
float motorPWMDuty = 0.0;
float encPWMDuty = 0.0;
float prevEncoderPWMDuty = 0.0;
float angularVelocity = 0.0;

// Velocity to reach.

float angleGoalDegrees = -10.0;
 
// Setup velocity PID controller
void initializePidController(void) {
    velocityPIDController.setInputLimits(-15.0, 15.0);
    velocityPIDController.setOutputLimits(-1.0, 1.0);
    velocityPIDController.setBias(0.0);
    velocityPIDController.setMode(PID_AUTO_MODE);
}
 
int main() {
 
    // Initialization
    initializePidController();

    wait(1);

    // Set velocity set point.
    velocityPIDController.setSetPoint(angleGoalDegrees);

    timer.start();

    float interval = 0.1;
    prevEncoderPWMDuty = pwmEncoder.avgDutyCycle();
 
    // Run
    while (true) {
        // Get the encoder PWM duty and calculate the angular velocity
        encPWMDuty = pwmEncoder.avgDutyCycle();
        angularVelocity = -360.0 * (encPWMDuty - prevEncoderPWMDuty) / interval; // Degrees per second
        prevEncoderPWMDuty = encPWMDuty;

        // Update the PID controller
        velocityPIDController.setInterval(interval);
        velocityPIDController.setProcessValue(angularVelocity);
        motorPWMDuty = velocityPIDController.compute();
        motor.setSpeed(motorPWMDuty);

        pc.printf("Angular Velocity: %f, \tGoal: %f, \tMotor DC: %f\r\n", angularVelocity, angleGoalDegrees, motorPWMDuty);

        wait(kUpdatePeriod);
        led = !led;

        interval = timer.read();
        timer.reset();
    }

}