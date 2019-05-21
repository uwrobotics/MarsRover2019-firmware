/*
 * UWaterloo Mars Rover 2018/19 Ultrasonic Sensor
 * Created by: Brad Stevanus
 * Hardware: https://www.robotshop.com/ca/en/weatherproof-ultrasonic-sensor-separate-probe.html
 */

#include "mbed.h"
#include "../../../lib/ultrasonic/ultrasonic.h"
#include "CANMsg.h"
#include "rover_config.h"

const double        UPDATE_INTERVAL = 0.1; // In seconds
const unsigned int  TIMEOUT = 1; // In seconds
int                 sonarDistance1;
int                 sonarDistance2;
Serial              pc(SERIAL_TX, SERIAL_RX);
CAN                 can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg              rxMsg;
CANMsg              txMsg;
DigitalOut          ledCAN(LED4);
Timer               canSendTimer; // For debugging CAN transmissions

void dist(int distance);
int main();

void dist(int distance) {
    // Put code here to execute when the distance changes if necessary
    pc.printf("distance: %d cm\r\n", distance);
}


/**
 * sonarObject 1 : trigPin = pin 7, echoPin = pin 8
 * sonarObject 2 : trigPin = pin 9, echoPin = pin 10
 */
ultrasonic sonarObject1(D7, D8, UPDATE_INTERVAL, TIMEOUT, &dist);
ultrasonic sonarObject2(D9, D10, UPDATE_INTERVAL, TIMEOUT, &dist);

// main() runs in its own thread in the OS
int main(void) {

    pc.printf("Program started\r\n");
    can.frequency(1000000); // Set bit rate to 1Mbps

    canSendTimer.start();

    // Start measuring the distance
    sonarObject1.startUpdates();
    sonarObject2.startUpdates();

    while (true) {

        sonarObject1.checkDistance();
        sonarObject2.checkDistance();
        sonarDistance1 = sonarObject1.getCurrentDistance();
        sonarDistance2 = sonarObject2.getCurrentDistance();

        if (canSendTimer.read_ms() >= 1000) {
            canSendTimer.stop();
            canSendTimer.reset();
            txMsg.clear();
            txMsg.id = ROVER_JETSON_CANID;
            txMsg << new int[2]{sonarDistance1, sonarDistance2};
            if (can.write(txMsg)) {
                ledCAN = 0;
                pc.printf("-------------------------------------\r\n");
                pc.printf("CAN message sent\r\n");
                printMsg(txMsg);
            } else {
                pc.printf("Transmission error\r\n");
            }
        }

    }
}
