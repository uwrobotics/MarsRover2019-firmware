/* UWaterloo Mars Rover 2018/19 Ultrasonic Sensor
 * Created by: Brad Stevanus
 * Hardware: https://www.robotshop.com/ca/en/weatherproof-ultrasonic-sensor-separate-probe.html
 */

#include "mbed.h"
#include "ultrasonic.h"

// Global variables
const double UPDATE_INTERVAL = 0.1; // in seconds
const unsigned int TIMEOUT = 1; // in seconds
int main();
void dist(int distance);
Serial pc(SERIAL_TX, SERIAL_RX); // Establish serial connection to PC

void dist(int distance) {
    // Put code here that executes when the distance is changed
    pc.printf("Distance changed to %d cm\r\n", distance);
}

// First and second parameters are trigPin and echoPin, respectively
ultrasonic sonarObject(D7, D8, UPDATE_INTERVAL, TIMEOUT, &dist);

// main() runs in its own thread in the OS
int main(void) {
    // Start measuring the distance
    sonarObject.startUpdates();
    while (true) {
    	  // Do something else here
        sonarObject.checkDistance();
    }
}
