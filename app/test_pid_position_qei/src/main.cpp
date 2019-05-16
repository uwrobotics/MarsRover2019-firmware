#include "PID.h"
#include "QEI.h"
#include "Motor.h"
#include "rover_config.h"

#define RATE 0.1

//Kc, Ti, Td, interval
PID controller(5.0, 0.0, 0.0, RATE);
#ifdef ROVERBOARD_ARM_PINMAP  
  QEI pv (ENCR1_CH1, ENCR1_CH2, ENCR1_INDEX, 360, QEI::X4_ENCODING);
  Motor clawMotor(MOTOR3, MOTOR3_DIR, ROVER_MOTOR_PWM_FREQ_HZ);
#else
  QEI pv(PB_0, PB_1, PB_2, 2048);  // update with science board-specific pin mappings from other branch
#endif

Serial pc(SERIAL_TX, SERIAL_RX);
PwmOut co(PB_13); // PWM_ELEV   // TODO science board pin mappings


const float A = 121;
const float B = -9.35E-05;
const float C = -1.11E-07;
const float D = 1.08E-12;

float PulseToMM(int pulse)
{
  return A + B*pulse + C*pow(pulse, 2) + D*pow(pulse, 3);
}


int cmd = 0;
float target = 0;
bool newData = false;
void onSerialRx()
{
  static char serialInBuffer[200]; 
  static int serialCount = 0;
  int a;
  float b;

  while (pc.readable()) { // in case two bytes are ready
    char byteIn = pc.getc();
    if ( (byteIn == 0x0D) || (byteIn == 0x0A))
    { // if an end of line is found
      // pc.printf("Print data %d %X \n\r", serialCount, byteIn);
      serialInBuffer[serialCount] == 0;                 // null terminate the input
      if (sscanf(serialInBuffer,"%d %f", &a, &b) == 2) { // managed to read all 3 values
        newData = true;
        cmd = a;
        target = b;
      }
      serialCount = 0; // reset the buffer
    } else {
      serialInBuffer[serialCount] = byteIn;            // store the character
      // pc.printf("Print data %d 0x%X \n\r", serialCount, byteIn);
      if (serialCount<200)                                      // increase the counter.
        serialCount++;
    }
  }
}
 

int main() {
  pc.baud(115200);
  pc.attach(&onSerialRx);

  //Analog input from 0.0 to 120mm
  controller.setInputLimits(0.0, 120);  // TODO input lims of qei encoder
  //Pwm output from -1.0 to 1.0
  controller.setOutputLimits(-1.0, 1.0);

  //If there's a bias.
  controller.setBias(0.0);
  controller.setMode(PID_AUTO_MODE);

  //We want the process variable to be 1.7V
  // controller.setSetPoint(50);  // TODO set point as qei encoder position


  int pulse = 0;
  float mm = 0;
  float pid = 0;
  bool running = false;

  while(1) {

    if (newData) {  
      newData = false;
      switch(cmd)
      {
        case 1:
          running = false;
          pv.reset();
          break;
        case 2:
          running = true;
          controller.setSetPoint(target);
          break;
        case 3:
          running = false;
          break;
        default:
          break;
      }
    }
 
    pulse = pv.getPulses();
    mm = PulseToMM(pulse);
    
    //Update the process variable.
    controller.setProcessValue(mm);

    //Set the new output.
    if (running)
    { 
      pid = controller.compute();
      clawMotor.speed(-pid);
    }

    pc.printf("Pulses is: %d -> %fmm -> %f\n\r", pulse, mm, pid);
    pc.printf("Cmd: %d Target: %f\n\r", cmd, target);
    //Wait for another loop calculation.
    wait(RATE);
  }

}
