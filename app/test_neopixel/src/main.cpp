// Code adopted from: https://github.com/bigjosh/SimpleNeoPixelDemo/blob/master/SimpleNeopixelDemo/SimpleNeopixelDemo.ino
// and https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/

#include "mbed.h"
#include "neopixel.h"

#define REPEAT_FIVE(a)     a;a;a;a;a
#define REPEAT_TEN(a)      REPEAT_FIVE(a); REPEAT_FIVE(a)

neopixel::neopixel(PinName signalPin, int pixel_num):led_signal(signalPin) {
	m_pixel_num = pixel_num;

	pin_mask = gpio_set(signalPin);

	uint32_t port_index = STM_PORT(signalPin);
	switch (port_index) {
	case PortA:
		pin_gpio = (GPIO_TypeDef *)(GPIOA_BASE);
		break;
	case PortB:
		pin_gpio = (GPIO_TypeDef *)(GPIOB_BASE);
		break;
	case PortC:
		pin_gpio = (GPIO_TypeDef *)(GPIOC_BASE);
		break;
	}

	pin_reg_set = &pin_gpio->BSRR;
	pin_reg_clr = &pin_gpio->BRR;
}

inline void neopixel::sendBit(int val) {

	if (val) {
		//*pin_reg_set |= pin_mask;
		led_signal = 1;
		wait(0.5);
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		led_signal = 0;
		wait(0.5);
		//*pin_reg_clr |= pin_mask;
		REPEAT_TEN(__NOP());
		REPEAT_TEN(__NOP());
		REPEAT_FIVE(__NOP());
	}
	else {
		//*pin_reg_set = pin_mask;
		led_signal = 1;
		wait(0.5);
		REPEAT_TEN(__NOP());
		REPEAT_FIVE(__NOP());
		__NOP();
		__NOP();

		//*pin_reg_clr = pin_mask;
		led_signal = 0;
		wait(0.5);
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

// Just wait long enough without sending any bits will cause the pixels to latch and display the last sent frame
void neopixel::show() {
	wait_us((RES / 1000UL) + 1);  // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}

void neopixel::showColor(unsigned char r, unsigned char g, unsigned char b) {
	for (int p = 0; p < m_pixel_num; p++) {
		sendPixel(r, g, b);
	}
	show();
}

int main(void) {
	neopixel neo(PA_5, 10);
	//DigitalOut led_signal(PA_5);
	//unsigned int pin_mask = gpio_set(PA_5);
	//GPIO_TypeDef* pin_gpio = (GPIO_TypeDef *)(GPIOA_BASE);
	//__IO uint32_t* pin_reg_set = &pin_gpio->BSRR;
	//__IO uint32_t* pin_reg_clr = &pin_gpio->BRR;

	while (1) {
	//	led_signal = 1;
	//	wait(0.5);
	//	led_signal = 0;
	//	wait(0.5);
		neo.sendBit(1);
	//	neo.showColor(0, 0, 255);
	//	*pin_reg_set = pin_mask;
	//	wait(1);
	//	*pin_reg_clr = pin_mask;
	//	wait(1);
	}
}
