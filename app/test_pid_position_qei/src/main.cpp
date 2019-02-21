#include "PID.h"
#include "QEI.h"

#define RATE 0.1

//Kc, Ti, Td, interval
PID controller(1.0, 0.0, 0.0, RATE);
QEI pv(PB_0, PB_1, PB_2, 2048);  // update with science board-specific pin mappings from other branch
PwmOut co(PB_13); // PWM_ELEV   // TODO science board pin mappings

int main() {

  //Analog input from 0.0 to 3.3V
  controller.setInputLimits(0.0, 5000);  // TODO input lims of qei encoder
  //Pwm output from 0.0 to 1.0
  controller.setOutputLimits(0.0, 1.0);
  //If there's a bias.
  controller.setBias(0.3);
  controller.setMode(PID_AUTO_MODE);
  //We want the process variable to be 1.7V
  controller.setSetPoint(2500);  // TODO set point as qei encoder position

  while(1) {
    //Update the process variable.
    controller.setProcessValue(pv.getPulses());
    //Set the new output.
    co = controller.compute();
    //Wait for another loop calculation.
    wait(RATE);
  }

}
