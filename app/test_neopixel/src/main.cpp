// Code adopted from: https://github.com/bigjosh/SimpleNeoPixelDemo/blob/master/SimpleNeopixelDemo/SimpleNeopixelDemo.ino
// and https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/

#include "mbed.h"
#include "gpio_api.h"

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

const int PIXEL_NUM = 10;

// initialize the pin 
//gpio_t ledSignal();
#ifdef NUCLEO_PINMAP
//gpio_init(&ledSignal, PA_5, PIN_OUTPUT);
DigitalOut ledSignal(PA_5);
#elif
//gpio_init(&ledSignal, PC_0, PIN_OUTPUT);
DigitalOut ledSignal(PC_0);
#endif

extern "C" int sendBitOne(); // assembly routine to send one bit of data
extern "C" int sendBitZero(); // assembly routine to send one bit of data

inline void sendBit(int val) {
	if (val) {
		//sendBitOne();
		/*
		*ledSignal.reg_set = ledSignal.mask;
		for (int i = 0; i < NS_TO_CYCLES(T1H); i++) {
			__NOP();
		}
		*ledSignal.reg_clr = ledSignal.mask;
		for (int i = 0; i < NS_TO_CYCLES(T1L); i++) {
			__NOP();
		}
		*/
		
	}
	else {
		//sendBitZero();
		/*
		*ledSignal.reg_set = ledSignal.mask;
		for (int i = 0; i < NS_TO_CYCLES(T0H); i++) {
			__NOP();
		}
		*ledSignal.reg_clr = ledSignal.mask;
		for (int i = 0; i < NS_TO_CYCLES(T0L); i++) {
			__NOP();
		}
		*/
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
	for (int p = 0; p < PIXEL_NUM; p++) {
		sendPixel(r, g, b);
	}
	show();
}
Serial pc(SERIAL_TX, SERIAL_RX);

int main(void) {
	//__NOP();
	unsigned int mask_pin5 = gpio_set(PA_5);
	volatile unsigned int *portA_set = (unsigned int *)0x4800001A;
	volatile unsigned int *portA_clr = (unsigned int *)0x48000028;
	while (1) {
		pc.printf("Port A address: %p\r\n", GPIOA_BASE);
		*portA_set |= mask_pin5;
		wait(0.5);

		*portA_clr |= mask_pin5;
		wait(0.5);
	}
}
