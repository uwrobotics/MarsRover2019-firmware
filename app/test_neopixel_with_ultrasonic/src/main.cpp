#include "mbed.h"
#include "CANMsg.h"
#include "rover_config.h"
#include "ultrasonic.h"
#include "neopixel.h"

Serial              pc(SERIAL_TX, SERIAL_RX);
#ifdef NUCLEO_PINMAP
CAN                 can(CAN_GPIO_RX_PIN, CAN_GPIO_TX_PIN, ROVER_CANBUS_FREQUENCY);
#else
CAN                 can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
#endif
CANMsg              rxMsg;
CANMsg              txMsg;
DigitalOut          ledErr(LED1);
DigitalOut          ledCAN(LED4);
Timer               canSendTimer; // For debugging CAN transmissions

const unsigned int  ULTRA_LEFT_TX_ID = 0x520;
const unsigned int  ULTRA_RIGHT_TX_ID = 0x521;
const unsigned int  NEO_STATE_RX_ID = 0x210;
const unsigned int  GREEN = 1;
const unsigned int  ORANGE = 2;
const unsigned int  RED = 3;

const double        UPDATE_INTERVAL = 0.1; // In seconds
const unsigned int  TIMEOUT = 1; // In seconds
int                 sonarDistanceLeft;
int                 sonarDistanceRight;

void dist(int distance);
void printMsg(CANMsg& msg);
void sendDistance(CANMsg& msg, int TX_ID, int distance);
void checkState(CANMsg& msg);
void selectColor(int color_index);
int main();

void dist(int distance) {
	// Put code here to execute when the distance changes if necessary
	pc.printf("distance: %d cm\r\n", distance);
}

#ifdef ROVERBOARD_SCIENCE_PINMAP
ultrasonic          sonarLeft(ULTRA_TRIG_1, ULTRA_ECHO_1, UPDATE_INTERVAL, TIMEOUT, &dist);
ultrasonic          sonarRight(ULTRA_TRIG_2, ULTRA_ECHO_2, UPDATE_INTERVAL, TIMEOUT, &dist);
neopixel            pixelStrip(NEO_PIXEL_SIGNAL, 13);
#else
ultrasonic          sonarLeft(D7, D8, UPDATE_INTERVAL, TIMEOUT, &dist);
ultrasonic          sonarRight(D9, D10, UPDATE_INTERVAL, TIMEOUT, &dist);
neopixel            pixelStrip(PA_5, 10);
#endif

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

void sendDistance(CANMsg& msg, int TX_ID, int distance) {
	msg.clear();
	msg.id = TX_ID;
	msg << distance;
	pc.printf("-------------------------------------\r\n");
	if (can.write(msg)) {
		ledCAN = !ledCAN;
		pc.printf("CAN message sent\r\n");
		printMsg(txMsg);
	}
	else {
		ledErr = 1;
		pc.printf("Transmission error for message with id %d\r\n");
	}
}

void checkState(CANMsg& msg) {
	if (can.read(msg)) {
		ledCAN = !ledCAN;
		if (msg.id == NEO_STATE_RX_ID) {
			pc.printf("CAN message received\r\n");
			printMsg(msg);
			sonarLeft.pauseUpdates();
			sonarRight.pauseUpdates();
			selectColor(rxMsg.data[0]);
		}
	}
	else {
		ledErr = 1;
		pc.printf("Reception error for message\r\n");
	}
}

void selectColor(int state) {
	switch (state) {
	case GREEN:
		pixelStrip.showColor(0, 255, 0);
		break;
	case ORANGE:
		pixelStrip.showColor(255, 127, 80);
		break;
	case RED:
		pixelStrip.showColor(255, 0, 0);
		break;
	}
}

// main() runs in its own thread in the OS
int main(void) {
	ledErr = 0;
	ledCAN = 0;

	canSendTimer.start();

	sonarLeft.startUpdates();
	sonarRight.startUpdates();

	while (true) {
		sonarLeft.checkDistance();
		sonarRight.checkDistance();
		sonarDistanceLeft = sonarLeft.getCurrentDistance();
		sonarDistanceRight = sonarRight.getCurrentDistance();

		if (canSendTimer.read_ms() >= 1000) {
			canSendTimer.stop();
			canSendTimer.reset();
			sendDistance(txMsg, ULTRA_LEFT_TX_ID, sonarDistanceLeft);
			sendDistance(txMsg, ULTRA_RIGHT_TX_ID, sonarDistanceRight);
		}

		checkState(rxMsg);
	}
}
