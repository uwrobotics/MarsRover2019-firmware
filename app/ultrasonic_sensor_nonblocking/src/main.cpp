/* UWaterloo Mars Rover 2018/19 Ultrasonic Sensor
 * Created by: Brad Stevanus
 * Hardware: https://www.robotshop.com/ca/en/weatherproof-ultrasonic-sensor-separate-probe.html
 */

#include "mbed.h"

// Global variables
Timer tmr;
DigitalOut trig(D5); // Pin 5 = trig pin
DigitalIn echo(D6);  // Pin 6 = echo pin
long duration; // Time interval that the echo pin spends high, in us (microseconds)
int distance; // Distance calculated based on formula (duration(us)/58)
int output; // output (main) = distance (get_distance)
int main();
int get_distance();
Serial pc(SERIAL_TX, SERIAL_RX); // Establish serial connection to PC

// main() runs in its own thread in the OS
int main(void) {
    tmr.start(); // Start timer
    while (true) {
    	output = get_distance();
        pc.printf("Distance: %d cm\r\n", output);
    }
}

int get_distance(void) {
    // Formula derived from Robotshop page
	// Initiate (10us TTL to signal pin)
	trig.write(1);
	wait_ms(10); // Waits for 10 microseconds
	trig.write(0);
	// Echo back
	while(echo.read() == 0); // Wait for echo pin to read high
	tmr.reset(); // Reset timer at start of echo signal
	while(echo.read() == 1); // Hold timer until echo outputs low
	duration = tmr.read_us();
	distance = duration/58; // Formula for distance in centimeters
	return distance;
}

// Resources: Robotshop ultrasonic sensor, STM32 Nucleo-64 boards pdf for pin names, mbed libraries, (REMOVE LATER)
