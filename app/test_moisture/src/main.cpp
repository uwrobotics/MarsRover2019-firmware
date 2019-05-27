#include "mbed.h"
#include "PinNames.h"
#include "moist.h"



AnalogIn moisture(MOIST_DATA);
DigitalOut moisture_pwr(MOIST_PWR);
Serial pc(SERIAL_TX, SERIAL_RX); // Uses default baud rate defined in config/mbed_config.h

int main() {
    moist_init(); // Init moisture

    while(1) { // loop for reading moisture value
    //?? should we include a filter?
        pc.printf("Moisture float percentage value is (%f)", moist_read());
        pc.printf("Moisture int value is (%f)", moist_read_u16());
        wait(1);
    }
}

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

