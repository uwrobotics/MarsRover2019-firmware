#include "mbed.h"
#include "CANMsg.h"
#include "rover_config.h"
#include "ultrasonic.h"
#include "neopixel.h"

Serial              pc(SERIAL_TX, SERIAL_RX);
#ifdef NUCLEO_PINMAP
CAN                 can(PB_8, PB_9, ROVER_CANBUS_FREQUENCY);
#else
CAN                 can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
#endif
CANMsg              rxMsg;
CANMsg              txMsg;
DigitalOut          ledErr(LED1);
DigitalOut			ledDebug(LED2);
DigitalOut          ledCANRX(LED3);
DigitalOut          ledCANTX(LED4);
Timer               canSendTimer; // For debugging CAN transmissions

const unsigned int  ULTRA_LEFT_TX_ID = 0x520;
const unsigned int  ULTRA_RIGHT_TX_ID = 0x521;
const unsigned int  NEO_STATE_RX_ID = 0x210;
const unsigned int  GREEN = 1;
const unsigned int  ORANGE = 2;
const unsigned int  RED = 3;

const float         UPDATE_INTERVAL = 0.1; // In seconds
const unsigned int  TIMEOUT = 1; // In seconds
const float			EXPO_FILTER_FACTOR = 0.2;
const int			RUNNING_AVERAGE_NUM = 20;
int				    running_average_buffer_left[RUNNING_AVERAGE_NUM] = { 0 };
int				    running_average_buffer_right[RUNNING_AVERAGE_NUM] = { 0 };
const int			DIST_LOWER_LIM = 25;   // when ultrasonic sensor is close to obstacles, readings are garbage
const int			DIST_HIGHER_LIM = 300; // according to https://www.robotshop.com/ca/en/weatherproof-ultrasonic-sensor-separate-probe.html#Useful-Links
volatile int        sonarDistanceLeft;
volatile int        sonarDistanceRight;
bool				arrived = false;

void printMsg(CANMsg& msg);
void CANSendDistance(CANMsg& msg, int TX_ID, int distance);
void CANCheckState(CANMsg& msg);
void selectColor(int color_index);
void dist_dummy(int distance);
float lowPassRunningAverage(int* buffer, int input, float average);
float lowPassExponential(int input, float average, float factor = EXPO_FILTER_FACTOR);
float capValue(int input, int lower_lim = DIST_LOWER_LIM, int upper_lim = DIST_HIGHER_LIM);
int main();

#ifdef ROVERBOARD_SCIENCE_PINMAP
ultrasonic          sonarLeft(ULTRA_TRIG_1, ULTRA_ECHO_1, UPDATE_INTERVAL, TIMEOUT, &dist_dummy);
ultrasonic          sonarRight(ULTRA_TRIG_2, ULTRA_ECHO_2, UPDATE_INTERVAL, TIMEOUT, &dist_dummy);
neopixel            pixelStrip(NEO_PIXEL_SIGNAL, 13);
#else
ultrasonic          sonarLeft(D7, D8, UPDATE_INTERVAL, TIMEOUT, &dist_dummy);
ultrasonic          sonarRight(D9, D10, UPDATE_INTERVAL, TIMEOUT, &dist_dummy);
neopixel            pixelStrip(PA_5, 10);
#endif

void dist_dummy(int distance) {
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

void CANSendDistance(CANMsg& msg, int TX_ID, int distance) {
	msg.clear();
	msg.id = TX_ID;
	msg << distance;
	pc.printf("-------------------------------------\r\n");
	if (can.write(msg)) {
		ledCANTX = !ledCANTX;
		pc.printf("CAN message sent\r\n");
		//printMsg(txMsg);
	}
	else {
		ledErr = 1;
		pc.printf("Transmission error for message with id 0x%.3x\r\n", TX_ID);
	}
}

void CANCheckState(CANMsg& msg) {
	if (can.read(msg)) {
		if (msg.id == NEO_STATE_RX_ID) {
			ledCANRX = !ledCANRX;
			printMsg(msg);
			sonarLeft.pauseUpdates();
			sonarRight.pauseUpdates();
			selectColor(msg.data[0]);
		}
	}
}

void selectColor(int state) {
	switch (state) {
	case GREEN:
		pixelStrip.showColor(0, 255, 0);
		arrived = true;
		break;
	case ORANGE:
		pixelStrip.showColor(255, 127, 80);
		arrived = true;
		break;
	case RED:
		pixelStrip.showColor(255, 0, 0);
		arrived = true;
		break;
	}
}

float lowPassExponential(int input, float average, float factor) {
	return input * factor + (1 - factor)*average;
}

float lowPassRunningAverage(int* buffer, int input, float average) {
	static int	count = 0;
	count++;
	if (count < RUNNING_AVERAGE_NUM*2) { // we have two buffers, so times 2. I'm too lazy to do it the proper way...
										 // @TODO: make a class for this
		return DIST_LOWER_LIM;
	}

	for (int i = RUNNING_AVERAGE_NUM - 1; i > 0; i--) {
		buffer[i] = buffer[i - 1];
	}
	buffer[0] = input;

	int sum = 0;
	for (int i = 0; i < RUNNING_AVERAGE_NUM; i++) {
		sum += buffer[i];
	}

	return sum / float(RUNNING_AVERAGE_NUM);
}

float capValue(int input, int lower_lim, int upper_lim) {
	if (input < lower_lim)
		return lower_lim;
	else if (input > upper_lim)
		return upper_lim;
	return input;
}

// main() runs in its own thread in the OS
int main(void) {
	canSendTimer.start();

	sonarLeft.startUpdates();
	sonarRight.startUpdates();

	while (true) {
		ledDebug = !ledDebug;
		if (!arrived) {
			//sonarLeft.checkDistance();
			//sonarRight.checkDistance();

			sonarDistanceLeft = capValue(lowPassRunningAverage(running_average_buffer_left, 
				                                               sonarLeft.getCurrentDistance(), 
				                                               sonarDistanceLeft));
			sonarDistanceRight = capValue(lowPassRunningAverage(running_average_buffer_right, 
				                                                sonarRight.getCurrentDistance(), 
				                                                sonarDistanceRight));

			if (canSendTimer.read_ms() >= 1000) {
				printf("dist lef : %d, dist right: %d \r\n", sonarDistanceLeft, sonarDistanceRight);

				canSendTimer.stop();
				canSendTimer.reset();
				canSendTimer.start();
				CANSendDistance(txMsg, ULTRA_LEFT_TX_ID, sonarDistanceLeft);
				CANSendDistance(txMsg, ULTRA_RIGHT_TX_ID, sonarDistanceRight);
			}
		}
	
		CANCheckState(rxMsg);
	}
}
