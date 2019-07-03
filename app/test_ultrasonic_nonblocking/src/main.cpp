/*
 * UWaterloo Mars Rover 2018/19 Ultrasonic Sensor
 * Created by: Brad Stevanus
 * Hardware: https://www.robotshop.com/ca/en/weatherproof-ultrasonic-sensor-separate-probe.html
 */

#include "mbed.h"
#include "ultrasonic.h"
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
void printMsg(CANMsg& msg);
int main();

void dist(int distance) {
    // Put code here to execute when the distance changes if necessary
    pc.printf("distance: %d cm\r\n", distance);
}

void printMsg(CANMsg& msg) {
	pc.printf("  ID      = 0x%.3x\r\n", msg.id);
	pc.printf("  Type    = %d\r\n", msg.type);
	pc.printf("  Format  = %d\r\n", msg.format);
	pc.printf("  Length  = %d\r\n", msg.len);
	pc.printf("  Data    =");
	for (int i = 0; i < msg.len; i++)
		pc.printf(" 0x%.2X", msg.data[i]);
	pc.printf("\r\n");
}


/**
 * sonarObject 1 : trigPin = pin 7, echoPin = pin 8
 * sonarObject 2 : trigPin = pin 9, echoPin = pin 10
 */
#ifdef ROVERBOARD_SCIENCE_PINMAP
ultrasonic          sonarObject1(ULTRA_TRIG_1, ULTRA_ECHO_1, UPDATE_INTERVAL, TIMEOUT, &dist);
ultrasonic          sonarObject2(ULTRA_TRIG_2, ULTRA_ECHO_2, UPDATE_INTERVAL, TIMEOUT, &dist);
#else
ultrasonic sonarObject1(D7, D8, UPDATE_INTERVAL, TIMEOUT, &dist);
ultrasonic sonarObject2(D9, D10, UPDATE_INTERVAL, TIMEOUT, &dist);
#endif

// main() runs in its own thread in the OS
int main(void) {

    pc.printf("Program started\r\n");
    canSendTimer.start();

    // Start measuring the distance
    sonarObject1.startUpdates();
    sonarObject2.startUpdates();

    while (true) {

        sonarObject1.checkDistance();
        sonarObject2.checkDistance();
        sonarDistance1 = sonarObject1.getCurrentDistance();
        sonarDistance2 = sonarObject2.getCurrentDistance();
		printf("dist1 : %d, dist2: %d \r\n", sonarDistance1, sonarDistance2);
		
        if (canSendTimer.read_ms() >= 1000) {
            canSendTimer.stop();
            canSendTimer.reset();
			canSendTimer.start();
            txMsg.clear();
            txMsg.id = ROVER_JETSON_CANID;
			txMsg << sonarDistance1 << sonarDistance2;
            if (can.write(txMsg)) {
                ledCAN = !ledCAN;
                pc.printf("-------------------------------------\r\n");
                pc.printf("CAN message sent\r\n");
                printMsg(txMsg);
            } else {
                pc.printf("Transmission error\r\n");
            }
        }
		wait_ms(100);
    }
}
