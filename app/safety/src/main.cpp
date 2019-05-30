#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"

const unsigned int  RX_ID = ROVER_SAFETY_CANID; 
const unsigned int  TX_ID = ROVER_JETSON_CANID + 30; 
const unsigned int  CAN_MASK = ROVER_CANID_FILTER_MASK;

I2C 				i2c(I2C_SDA, I2C_SCL);
Serial              pc(SERIAL_TX, SERIAL_RX, ROVER_DEFAULT_BAUD_RATE);
CAN                 can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg              txMsg;

DigitalOut          ledErr(LED1);
DigitalOut          ledTest(LED2);
DigitalOut			ledI2C(LED3);
DigitalOut          ledCAN(LED4);
AnalogIn            battery(V_MONITOR);

//Sensor Address Indices
enum Sensor_Index{
	I_100A1 = 0, 
	I_100A2 = 1, 
	I_30A = 2,
};
enum Sensor_Address{
	A_100A1 = 0x54, 
	A_100A2 = 0x50, 
	A_30A = 0x61,
};

 
#define TOTAL_SAMPLE 10
//char adc_sample[2]; 
float raw_adc_sum = 0.0;
float raw_adc[3];
float current[3];

// Specfic variables for 100A sensor
const float lowerV_100A = 0.314; //lowerV and upperV is the voltage after signal has been amplified
const float upperV_100A = 3.218;
const float ampRange_100A = 100;
const float bitsPerVolt_100A = 255/3.3;
const float bitRange_100A = (upperV_100A - lowerV_100A) * bitsPerVolt_100A; // 224.4
const float iConversion_100A = bitRange_100A/ampRange_100A; // 2.244

// Specfic variables for 30A sensor
const float lowerV_30A = 0.385714; //lowerV and upperV is the voltage after signal has been amplified
const float upperV_30A = 4.785714;
const float ampRange_30A = 30;
const float bitsPerVolt_30A = 255/5;
const float bitRange_30A = (upperV_30A - lowerV_30A) * bitsPerVolt_30A;
const float iConversion_30A = bitRange_30A/ampRange_30A;


//-----
float new_reading = 0;
float C_100A1_avg = 0;
float C_100A2_avg = 0;
float C_30A_avg = 0;
char raw_adc_value = 0;

// Voltage monitoring specific
const float compression_factor = 0.110629;
const int cell_num = 6;
const float full_cell = 4.20;
const float low_cell = 3.75; // TODO verify this value
const float full_bat = full_cell * cell_num;
const float low_bat = low_cell * cell_num;
const float board_voltage = 3.3;
float bat_value = 0;
float bat_value_raw = 0;
float bat_avg = 0;
#define V_INDEX 3

void initCAN() {
    can.filter(RX_ID, ROVER_CANID_FILTER_MASK, CANStandard);

    // for (int canHandle = firstCommand; canHandle <= lastCommand; canHandle++) {
    //     can.filter(RX_ID + canHandle, 0xFFF, CANStandard, canHandle);
    // }
}

/*
Read the current sensor values using I2C into current
@param	address: the I2C address for ADC on the current sensor
@return None
*/
float read_current(Sensor_Address address){
	int status;
	float current = 0;
	char adc_sample[2]; 
	status = i2c.read( (address<<1), &adc_sample[0], 2, false);
	if (!status) // error
		pc.printf("I2C ERROR\n");
	raw_adc_value = (adc_sample[0] << 4) + (adc_sample[1] >> 4);
	pc.printf("!!Adc sample[0] %x \t acd sample[1] %x \n", adc_sample[0], adc_sample[1]);
	pc.printf("ADC value merged: %x\n", raw_adc_value);
	ledI2C = !ledI2C;
	// conversion to current
	if (address == A_100A1){
		// pc.printf("value test: %x \n", (246 - raw_adc_value));
		current = ((246 - raw_adc_value)/iConversion_100A) - 16.5;
		}
	else if (address == A_100A2)
		current = ((246 - raw_adc_value)/iConversion_100A) - 1;
	else
		current = ((bitRange_30A - raw_adc_value)/iConversion_30A);
	// output reading
	pc.printf("Address: 0x%.8X \tCurrent: %f\r\n", address, current);
	return current;
}

/*
Send out averaged current value over CAN
@param	address: the I2C address for ADC on the current sensor
@return None
*/
void CAN_send(float value, Sensor_Index can_index){
	uint8_t data = value; // Convert current data to int 
	CANMsg txMsg(TX_ID + can_index);
	txMsg << data; // append data to CANMsg
	ledCAN = !ledCAN;
	if(can.write(txMsg)) {
		pc.printf("Sucessful CAN transmission\r\n");
	}
	else {
		pc.printf("Error in CAN transmission\r\n");
		ledErr = 1;
	}
}


int main() {
	pc.printf("Program Started\r\n\r\n");
	
	i2c.frequency(100000);
    initCAN();
	ledI2C = 1;
	ledCAN = 1;
	int i = 1;

    while(1) {
		//raw_adc_sum = 0.0;
		
        /* avgReading -= avgReading/sampleSize;
           avgReading += newReading/sampleSize
        */

	   /* for each current sensor :
	   *    if sensor_100A1:
	   * 		have case for when readings haven't reached TOTAL_SAMPLE
	   * 		100A1_avg -= 100A1_avg/TOTAL_SAMPLE
	   * 		100A1_avg += newreading/TOTAL_SAMPLE
	   */
		// Take sensor 1 reading
		new_reading = read_current(A_100A1);
		C_100A1_avg -= C_100A1_avg/TOTAL_SAMPLE;
		C_100A1_avg += new_reading/TOTAL_SAMPLE;
		CAN_send(C_100A1_avg, I_100A1);

		// // Take sensor 2 reading
		read_current(A_100A2);
		C_100A2_avg -= C_100A2_avg/TOTAL_SAMPLE;
		C_100A2_avg += new_reading/TOTAL_SAMPLE;
		CAN_send(C_100A2_avg, I_100A2);

		// // Take sensor 3 reading
		read_current(A_30A);
		C_30A_avg -= C_30A_avg/TOTAL_SAMPLE;
		C_30A_avg += new_reading/TOTAL_SAMPLE;
		CAN_send(C_30A_avg, I_30A);


        // Reading of battery voltage
        /* Battery voltage conversion 
        * Voltage divider of 82k and 10.2k provide a compression factor of 0.110629
		* 10.19k, 81.8k
        * Does CAN Msg need to be sent out?
        * 400 mV when 5V is on
        * 0.16 reading
        */

       bat_value_raw = battery.read();
       bat_value = bat_value_raw * board_voltage/ compression_factor;
		bat_avg -= bat_avg/TOTAL_SAMPLE;
		bat_avg += bat_value/TOTAL_SAMPLE;
       pc.printf("Battery Level: %f\r\n", bat_avg);
       if (bat_value < low_cell){
		   // TODO: send out CAN here
           // CANMsg txMsg(TX_ID + V_INDEX);
           pc.printf("REPLACE BATTERY, Battery Level LOW!!! \r\n");
       }
        pc.printf("Hello World! %d\r\n", i);
		// TODO: send out CAN here with proper battery voltage

        i++;
        ledTest = i % 2;
		wait(1);
    }
}
