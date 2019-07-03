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

PID velocityPIDController(5.5, 0.0, 0.0, kUpdatePeriod);
Timer timer;

// Variables
float motorPWMDuty = 0.0;
float jointAngle = 0.0;

// Velocity to reach.

float angleGoalDegrees = 0.0;

// Setup velocity PID controller
void initializePidController(void) {
    velocityPIDController.setInputLimits(-180, 180.0);
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

    float interval = 0.00;

    // Run
    while (true) {
        // Get the encoder PWM duty and calculate the angular velocity
        jointAngle = -(pwmEncoder.avgDutyCycle() - 0.5) * 360.0;
        interval = timer.read();
        timer.reset();

        // Update the PID controller
        velocityPIDController.setInterval(interval);
        velocityPIDController.setProcessValue(jointAngle);
        motorPWMDuty = velocityPIDController.compute();
        motor.setDutyCycle(motorPWMDuty);

        pc.printf("Angle: %f, \tGoal: %f, \tMotor DC: %f\r\n", jointAngle, angleGoalDegrees, motorPWMDuty);

        wait(kUpdatePeriod);
        led = !led;
    }

}