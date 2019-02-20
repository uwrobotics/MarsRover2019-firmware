#include "mbed.h"
#include "pot.h"
#include "servo.h"
#include "pid.h"

// HSR-1425CR
// Define constants

static const uint16_t PERIOD = 2500; // 2500us seems to give less jitter
static const float MIN_PID_CONTROL = -1500;
static const float MAX_PID_CONTROL = 989;
static const float EPSILON = 5; 

// Initialise objects

Pot pot(A0); //potentiometer
Pot testPot(A1);
Servo servoA(D9, PERIOD); //continuous servo
Servo servoB(D11, PERIOD); //non-continuous servo
Pid pid(MIN_PID_CONTROL, MAX_PID_CONTROL); //pid controller
Serial pc(USBTX, USBRX); //serial object
DigitalOut led1(LED1);


/**
 * @brief Moves non-continous servo to desired angle.
 * Only needs to be called once
 *
 * @param degrees -> servo target in degrees
 */


/**
 * @brief Moves continuous servo to desired location.
 * The function is non-blocking, so it should be called
 * as often until it reaches target. Each call should have
 * a delay between them.
 *
 * @param degrees -> servo target in degrees
 * @return int16_t -> -1 : failed to write
 *                     0 : not reached target
 *                     1 : reached target
 */


void nonContinuousTest(float angle)
{
	servoB.writeDegreesNCON(angle);
}

float potReadTest()
{
   while(1){

	float val = (pot.readPos()/100)/3.14;
	pc.printf("The Angle %f\r\n", val);
   }

}
/*void test(){
	bool check=false;
	while(1){
        float timenow= pid.controller(151, pot);
		pc.printf("The time %f\r\n", timenow);
	    wait_ms(100);
		}
}*/
void continuous(float angle){
	bool check =false;

	while(!check){
		led1 = 0;
		float x = pid.controller(angle, pot);

	    servoA.writeMicroseconds(1510+x);
		wait_ms(100);
		servoA.writeMicroseconds(1510);
        wait_ms(10);

		if (x == 0){
			led1 = 1;
			check=true;
		}
		
		pc.printf("The Angle %f\r\n", pot);
		//pc.printf("The X %f\r\n", x);
	}
}
#ifdef __cplusplus
extern "C" {
#endif
//#include "canlib.h"
//#include "pins.h"
volatile float TangleCON = 40;   
volatile float TangleNCON = 45;
// main() runs in its own thread in the OS 1470 backwards and 1570 forwards
int main()
{
  /*
    if (CANLIB_Init(20, 1) != 0)
    {
        pc.printf("init failed\r\n");
    }
    if (CANLIB_AddFilter(410) != 0)
    {
        pc.printf("add filter failed\r\n");
    }*/

   // while(true){servoA.writeMicroseconds(1554);wait(2.5); servoA.writeMicroseconds(1510); potReadTest();} //Testing not of any use.
	
  	//nonContinuousTest(TangleNCON);
	float val;
	while(true){
		//val = (testPot.readPos()/100.0)/1.87;
		val = 0;
		continuous(val);
	}

	//float val = ((testPot.readPos())/65536.0)*360;
	//continuous(val);
 // potReadTest();

  return 0;
}
/*
void CANLIB_Rx_OnMessageReceived(void)
{
	TangleCON= CANLIB_Rx_GetAsInt(CANLIB_INDEX_0);

	TangleNCON= CANLIB_Rx_GetAsInt(CANLIB_INDEX_1);

}*/
#ifdef __cplusplus
}
#endif
