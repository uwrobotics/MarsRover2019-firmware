#include "mbed.h"
#include "Servo.h"

DigitalOut led(LED1);
Serial pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h
Servo myservo(SERVO_P);

int main() {
    while(1) {
        myservo = 0.3;
        wait(1);
        myservo = 0.55;
        wait(1);
        myservo = 0.7;
        wait(1);
        myservo = 0.55;
        wait(1);
//        for(float p = 0.4; p < 0.6; p += 0.02) {
//            myservo = p;
//            pc.printf("Set servo position to %f\r\n", p);
//            wait(1);
//        }
    }

}