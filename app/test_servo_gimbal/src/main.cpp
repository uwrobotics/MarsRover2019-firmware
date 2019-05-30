#include "mbed.h"
#include "PinNames.h"
#include "Servo.h"

DigitalOut ledA(LED1);
// DigitalIn  button1(BUT_1);
// DigitalIn  button2(BUT_2);
//Servo servo_yaw(SERVO_F);
Servo servo_yaw(SERVO_YAW);
Servo servo_pitch(SERVO_PITCH);
Serial pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h

void moveRight(void){
        servo_yaw = 0.2;
        wait(0.2);
        servo_yaw = 0.50;
        wait(1);
}
void moveLeft(void){
        servo_yaw = 0.8;
        wait(0.2);
        servo_yaw = 0.50;
        wait(1);
}

void moveUp(void){
        servo_pitch = 0.4;
        wait(0.2);
        servo_pitch = 0.50;
        wait(1);
}
void moveDown(void){
        servo_pitch = 0.6;
        wait(0.2);
        servo_pitch = 0.50;
        wait(1);
}

int main() {
        ledA = 1;

    while(1) {
        // servo_pitch = 0.001;
        // wait(2);
        // servo_pitch = 0.501;
        // wait(2);

        //servo_pitch.position(-20);
        // servo_pitch.position(20);
        // wait(2);
        // servo_pitch.position(30);
        // wait(2);
        // servo_pitch.position(20);
        // wait(2);
        // servo_pitch.position(40);
        // wait(2);
        // moveUp();
        // //wait(1);
        // moveUp();
        // //wait(1);
        // moveDown();
        // //wait(1);
        // moveDown();
        //wait(1);
        moveRight();
        //wait(1);
        moveRight();
        //wait(1);
        moveRight();
        //wait(1);
        moveLeft();
        moveLeft();
        //wait(1);
        moveLeft();
        // wait(1);
        ledA=!ledA;
    }

}