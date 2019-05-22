#include "mbed.h"
#include "neopixel.h"

#ifdef NUCLEO_PINMAP 
neopixel neo(PA_5, 10);
#else 
neopixel neo(PC_0, 10);
#endif

const int DELAY_TIME_S = 1;

int main(void) {
	while (1) {
		neo.showColor(255, 0, 0);
		wait(DELAY_TIME_S);
		neo.showColor(0, 255, 0);
		wait(DELAY_TIME_S);
		neo.showColor(255, 127, 80);
		wait(DELAY_TIME_S);
	}
}