// Code adopted from: https://github.com/bigjosh/SimpleNeoPixelDemo/blob/master/SimpleNeopixelDemo/SimpleNeopixelDemo.ino
// and https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/

#include "mbed.h"
#include "CANMsg.h"
#include "rover_config.h"

#define REPEAT_FIVE(a)     a;a;a;a;a
#define REPEAT_TEN(a)      REPEAT_FIVE(a); REPEAT_FIVE(a)

// CPU cycle timing
#define NS_PER_SEC (1000000000L) 
#define CYCLES_PER_SEC (48000000.0)
#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )

#define NS_TO_CYCLES(n) ( int((n) / NS_PER_CYCLE / 6.0 ))

// Timings for 0 and 1 bit
#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns

#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns

#define RES 6000    // Width of the low gap between bits to cause a frame to latch

#ifdef NUCLEO_PINMAP
DigitalOut			led_signal(PA_5);
#else
DigitalOut			led_signal(PC_0);
CAN                 can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg              rxMsg;
const unsigned int  RX_ID = 0x210;
const unsigned int  GREEN = 1;
const unsigned int  ORANGE = 2;
const unsigned int  RED = 3;
#endif

const int			m_pixel_num = 10;

inline void sendBit(int val) {

	if (val) {
		led_signal = 1;
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());

		led_signal = 0;
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		REPEAT_FIVE(__NOP());
	}
	else {
		led_signal = 1;
		REPEAT_TEN(__NOP());
		__NOP();
		__NOP();

		led_signal = 0;
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
	}
	
}

inline void sendByte(unsigned char byte) {

	for (unsigned char bit = 0; bit < 8; bit++) {

		sendBit((byte >> bit) & 1);  // Neopixel needs bit in highest-to-lowest order
									// so send highest bit (bit #7 in an 8-bit byte since they start at 0)
		byte <<= 1;                // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
	}
}

inline void sendPixel(unsigned char r, unsigned char g, unsigned char b) {

	sendByte(g);          // Neopixel needs colors in green then red then blue order
	sendByte(r);
	sendByte(b);

}

// Just wait long enough without sending any bits will cause the pixels to latch and display the last sent frame
void show() {
	wait_us((RES / 1000UL) + 1);  // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}

void showColor(unsigned char r, unsigned char g, unsigned char b) {
	for (int p = 0; p < m_pixel_num; p++) {
		sendPixel(r, g, b);
	}
	show();
}


int main(void) {

	while (1) {
#ifdef ROVERBOARD_SCIENCE_PINMAP
		if (can.read(rxMsg)) {
			// Filtering performed by software:
			if (rxMsg.id == RX_ID) {
				int color_state = rxMsg.data[0];
				switch (color_state) {
				case GREEN:
					showColor(0, 255, 0);
					break;
				case ORANGE:
					showColor(255, 127, 80);
					break;
				case RED:
					showColor(255, 0, 0);
					break;
				}
			}
		}
#else
		showColor(255, 0, 0);
		wait(1);
		showColor(0, 255, 0);
		wait(1);
		showColor(255, 127, 80);
		wait(1);
#endif
	}
}
