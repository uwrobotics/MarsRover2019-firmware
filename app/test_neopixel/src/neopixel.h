#ifndef MBED_NEOPIXEL_H
#define MBED_NEOPIXE_H

#include "mbed.h"
#include "gpio_api.h"

class neopixel
{
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

public:
	neopixel(PinName signalPin, int pixel_num);
	//extern "C" int sendBitOne(); // assembly routine to send one bit of data
	//extern "C" int sendBitZero(); // assembly routine to send one bit of data
	inline void sendBit(int val);
	inline void sendByte(unsigned char byte);
	inline void sendPixel(unsigned char r, unsigned char g, unsigned char b);
	void show();
	void showColor(unsigned char r, unsigned char g, unsigned char b);
private:
	int m_pixel_num;
	DigitalOut led_signal;
	unsigned int pin_mask;
	GPIO_TypeDef* pin_gpio;
	__IO uint32_t *pin_reg_set;
	__IO uint32_t *pin_reg_clr;
	
};
#endif