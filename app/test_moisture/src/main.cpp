#include "mbed.h"
#include "PinNames.h"
#include "moist.h"


AnalogIn        moisture(MOIST_DATA);
DigitalOut      moisture_pwr(MOIST_PWR);
Serial          pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h

DigitalOut ledA(LED1);
DigitalOut ledB(LED2);
DigitalOut ledC(LED3);
DigitalOut ledD(LED4);


void moist_init(void){
    moisture_pwr = 0;
}

float moist_read(void){
    float value = 0;
    moisture_pwr = 1;
    wait_ms(10); // according to the sparkfun example
    value = moisture.read();
    moisture_pwr = 0;
    return value;
}

unsigned short moist_read_u16(void){
    short value = 0;
    moisture_pwr = 1;
    wait_ms(10); // according to the sparkfun example
    value = moisture.read_u16();
    moisture_pwr = 0;
    return value;
}



int main() {
    float tempVal;
    unsigned short tempValU16;
    moist_init();
    pc.printf("Moisture sensor start\n");
    while(1) { // loop for reading moisture value
               //?? should we include a filter?
        ledA = 1;
        ledB = 0;
        ledC = 1;
        ledD = 0;
        tempVal = moist_read();
        tempValU16 = moist_read_u16();
        pc.printf("Moisture float percentage value is (%.2f)\n", tempVal);
        pc.printf("Moisture int value is (%d)\n", tempValU16);
        wait(1);
    }
}

