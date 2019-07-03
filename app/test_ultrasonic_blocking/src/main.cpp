/* UWaterloo Robotics - Mars Rover 2018/19 Ultrasonic Sensor
 * Created by: Brad Stevanus
 * Hardware: https://www.robotshop.com/ca/en/weatherproof-ultrasonic-sensor-separate-probe.html
 */

#include "mbed.h"

// Global variables
const double UPDATE_INTERVAL = 0.1; // in seconds
const unsigned int HIGH = 1;
const unsigned int LOW = 0;
Timer tmr;
#ifdef ROVERBOARD_SCIENCE_PINMAP
DigitalOut trig(ULTRA_TRIG_1);
DigitalIn echo(ULTRA_ECHO_1);
#else
DigitalOut trig(D7);
DigitalIn echo(D8);
#endif
long duration; // Time interval associated with echo pulse, in us (microseconds)
int distance; // Distance calculated based on formula (duration(us)/58) (cm)
int output;
int main();
int get_distance();
Serial pc(SERIAL_TX, SERIAL_RX); // Establish serial connection to PC

// main() runs in its own thread in the OS
int main(void) {
    // Start timer
    tmr.start();
    while (true) {
    	  output = get_distance();
        pc.printf("Distance: %d cm\r\n", output);
        wait(UPDATE_INTERVAL);
    }
}

int get_distance(void) {
    // Set trigger pin to low for 2us
    trig.write(LOW);
    wait_ms(2);

  	// Send a 10us pulse to trigger for initialization
  	trig.write(HIGH);
  	wait_ms(10);
  	trig.write(LOW);

  	// Read echo pulse
  	while(echo.read() == LOW); // Wait for echo pin to read high
  	tmr.reset(); // Reset timer at start of echo signal
  	while(echo.read() == HIGH); // Hold timer until echo outputs low
  	duration = tmr.read_us();
  	distance = duration/58; // Formula for distance in centimeters
  	return distance;
}
