#include "neopixel.h"

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


neopixel::neopixel(PinName signalPin, int pixel_num):led_signal(signalPin) {
	m_pixel_num = pixel_num;
}

inline void neopixel::sendBit(int val) {
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

inline void neopixel::sendByte(unsigned char byte) {
	for (unsigned char bit = 0; bit < 8; bit++) {

		sendBit((byte >> bit) & 1);  // Neopixel needs bit in highest-to-lowest order
									// so send highest bit (bit #7 in an 8-bit byte since they start at 0)
		byte <<= 1;                // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
	}
}

inline void neopixel::sendPixel(unsigned char r, unsigned char g, unsigned char b) {
	sendByte(g);          // Neopixel needs colors in green then red then blue order
	sendByte(r);
	sendByte(b);
}

void neopixel::show() {
	wait_us((RES / 1000UL) + 1);  // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}

void neopixel::showColor(unsigned char r, unsigned char g, unsigned char b) {
	__disable_irq();
	for (int p = 0; p < m_pixel_num; p++) {
		sendPixel(r, g, b);
	}
	show();
	__enable_irq();
}

void neopixel::detonate(unsigned char r, unsigned char g, unsigned char b, unsigned int startdelayms) {
	while (startdelayms) {
		showColor(r, g, b);      // Flash the color 
		showColor(0, 0, 0);

		wait_ms(startdelayms);

		startdelayms = (startdelayms * 4) / 5; // delay between flashes is halved each time until zero
	}

	// Then we fade to black....
	for (int fade = 256; fade > 0; fade--) {
		showColor((r * fade) / 256, (g*fade) / 256, (b*fade) / 256);
	}

	showColor(0, 0, 0);
}