#include "PID.h"
 
#define RATE 0.1
 
//Kc, Ti, Td, interval
PID controller(1.0, 0.0, 0.0, RATE);
AnalogIn pv(p15);
PwmOut   co(p26);
 
int main(){
 
  //Analog input from 0.0 to 3.3V
  controller.setInputLimits(0.0, 3.3);
  //Pwm output from 0.0 to 1.0
  controller.setOutputLimits(0.0, 1.0);
  //If there's a bias.
  controller.setBias(0.3);
  controller.setMode(PID_AUTO_MODE);
  //We want the process variable to be 1.7V
  controller.setSetPoint(1.7);
 
  while(1){
    //Update the process variable.
    controller.setProcessValue(pv.read());
    //Set the new output.
    co = controller.compute();
    //Wait for another loop calculation.
    wait(RATE);
  }
 
}
