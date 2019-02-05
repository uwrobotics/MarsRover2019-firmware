#include "mbed.h"

DigitalOut ledA(LED1);
DigitalOut ledB(LED2);
DigitalOut ledC(LED3);
DigitalOut ledD(LED4);

int main() {
	int count = 0;

    while(1) {

    	// Trigger the four LEDs one after the other
    	switch (count) {
    		case 1: 
    			ledA = ;
    			ledD = 0;
    			break;
    		case 2:
    			ledB = 1;
    			ledA = 0;
    			break;
    		case 3:
    			ledC = 1;
    			ledB = 0;
                break;
    		case 4:
    			ledD = 1;
    			ledC = 0;
    			count = 0;
    			break;
    	}

    	count++;
        wait(0.5);
    }
}