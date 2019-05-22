/*
The idea behind this implementation is adopted from: 
https://github.com/bigjosh/SimpleNeoPixelDemo/blob/master/SimpleNeopixelDemo/SimpleNeopixelDemo.ino
and https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/

Hardware: https://cdn-shop.adafruit.com/datasheets/WS2812.pdf
*/

#ifndef MBED_NEOPIXEL_H
#define MBED_NEOPIXEL_H

#include "mbed.h"

class neopixel
{
public:
	/**iniates the class with the specified signal pin and number of neopixels in the strip**/
	neopixel(PinName signalPin, int pixel_num);
	/**send one bit of data**/
	inline void sendBit(int val);
	/**send one byte of data**/
	inline void sendByte(unsigned char byte);
	/**send one pixel of data (RGB)**/
	inline void sendPixel(unsigned char r, unsigned char g, unsigned char b);
	/**signals to the neopixel that the complete signal has been send, start displaying**/
	void show();
	/**display a single color on the whole strip**/
	void showColor(unsigned char r, unsigned char g, unsigned char b);
	/**flashes get faster and faster until *boom* and fade to black**/
	void detonate(unsigned char r, unsigned char g, unsigned char b, unsigned int startdelayms);
	
private:
	DigitalOut led_signal;
	int m_pixel_num;
};

#endif
