#include "mbed.h"
#include "rover_config.h"
#include "CANMsg.h"
#include "PwmIn.h"
#include "PID.h"
#include "Motor.h"

const unsigned int  RX_ID = 0x400; 
const unsigned int  TX_ID = ROVER_JETSON_CANID; 
const unsigned int  CAN_MASK = ROVER_CANID_FILTER_MASK;

const float ANGULAR_VELOCITY_INPUT_MIN = 0.0;
const float ANGULAR_VELOCITY_INPUT_MAX = 200.0;

const float MOTOR_OUTPUT_MIN = -0.3;
const float MOTOR_OUTPUT_MAX = 0.3;

// const float MOTOR1_VELOCITY_PID_P = 0.0;
// const float MOTOR1_VELOCITY_PID_I = 0.0;
// const float MOTOR1_VELOCITY_PID_D = 0.0;

// const float MOTOR2_VELOCITY_PID_P = 0.0;
// const float MOTOR2_VELOCITY_PID_I = 0.0;
// const float MOTOR2_VELOCITY_PID_D = 0.0;

// const float MOTOR3_VELOCITY_PID_P = 0.0;
// const float MOTOR3_VELOCITY_PID_I = 0.0;
// const float MOTOR3_VELOCITY_PID_D = 0.0;

// const float PID_UPDATE_INTERVAL = 0.01;

bool directMotorControlEnabled = true;
// bool velocityPIDControlEnabled = false;

double jointAngle[3];

Serial              pc(SERIAL_TX, SERIAL_RX, ROVER_DEFAULT_BAUD_RATE);
CAN                 can(CAN_RX, CAN_TX, ROVER_CANBUS_FREQUENCY);
CANMsg              rxMsg;
CANMsg              txMsg;

DigitalOut          ledErr(LED1);
DigitalOut          ledCAN(LED4);

Motor               motor1(MOTOR1, MOTOR1_DIR, ROVER_MOTOR_PWM_FREQ_HZ);
Motor               motor2(MOTOR2, MOTOR2_DIR, ROVER_MOTOR_PWM_FREQ_HZ);
Motor               motor3(MOTOR3, MOTOR3_DIR, ROVER_MOTOR_PWM_FREQ_HZ);
PwmIn               absEnc1(ENC_A1);
PwmIn               absEnc2(ENC_A2);
PwmIn               absEnc3(ENC_A3);

Timer               canSendTimer; 

// PID motor1PIDController(MOTOR1_VELOCITY_PID_P, MOTOR1_VELOCITY_PID_I, MOTOR1_VELOCITY_PID_D, PID_UPDATE_INTERVAL);
// PID motor2PIDController(MOTOR2_VELOCITY_PID_P, MOTOR2_VELOCITY_PID_I, MOTOR2_VELOCITY_PID_D, PID_UPDATE_INTERVAL);
// PID motor3PIDController(MOTOR3_VELOCITY_PID_P, MOTOR3_VELOCITY_PID_I, MOTOR3_VELOCITY_PID_D, PID_UPDATE_INTERVAL);

// enum pidControlMode {
//     velocityPID,
//     positionPID
// };

// Setup velocity PID controller
// void initializePidControllers(pidControlMode controlMode) {
    
//     if (controlMode == velocityPID) {
        
//         // motor1IDController = new PID(MOTOR1_VELOCITY_PID_P, MOTOR1_VELOCITY_PID_I, MOTOR1_VELOCITY_PID_D, PID_UPDATE_INTERVAL);
//         // motor1IDController = new PID(MOTOR2_VELOCITY_PID_P, MOTOR2_VELOCITY_PID_I, MOTOR2_VELOCITY_PID_D, PID_UPDATE_INTERVAL);
//         // motor1IDController = new PID(MOTOR3_VELOCITY_PID_P, MOTOR3_VELOCITY_PID_I, MOTOR3_VELOCITY_PID_D, PID_UPDATE_INTERVAL);

//         motor1PIDController.setInputLimits(ANGULAR_VELOCITY_INPUT_MIN, ANGULAR_VELOCITY_INPUT_MIN);
//         motor2PIDController.setInputLimits(ANGULAR_VELOCITY_INPUT_MIN, ANGULAR_VELOCITY_INPUT_MIN);
//         motor3PIDController.setInputLimits(ANGULAR_VELOCITY_INPUT_MIN, ANGULAR_VELOCITY_INPUT_MIN);

//     }

//     motor1PIDController.setOutputLimits(MOTOR_OUTPUT_MIN, MOTOR_OUTPUT_MAX);
//     motor1PIDController.setMode(PID_AUTO_MODE);
//     motor2PIDController.setOutputLimits(MOTOR_OUTPUT_MIN, MOTOR_OUTPUT_MAX);
//     motor2PIDController.setMode(PID_AUTO_MODE);
//     motor3PIDController.setOutputLimits(MOTOR_OUTPUT_MIN, MOTOR_OUTPUT_MAX);
//     motor3PIDController.setMode(PID_AUTO_MODE);
// }

void printCANMsg(CANMessage& msg) {
    pc.printf("  ID      = 0x%.3x\r\n", msg.id);
    pc.printf("  Type    = %d\r\n", msg.type);
    pc.printf("  Format  = %d\r\n", msg.format);
    pc.printf("  Length  = %d\r\n", msg.len);
    pc.printf("  Data    =");
    for(int i = 0; i < msg.len; i++)
        pc.printf(" 0x%.2X", msg.data[i]);
    pc.printf("\r\n");
}

enum armCommand {
 
    configureDirectMotorControl = RX_ID,
    setSpeedMotor1,
    setSpeedMotor2,
    setSpeedMotor3,
    // configureVelocityPIDControl,
    // setVelocityJoint1,
    // setVelocityJoint2,
    // setVelocityJoint3,

    firstCommand = configureDirectMotorControl,
    lastCommand  = setSpeedMotor3

};

void initCAN() {
    can.filter(RX_ID, ROVER_CANID_FILTER_MASK, CANStandard);

    // for (int canHandle = firstCommand; canHandle <= lastCommand; canHandle++) {
    //     can.filter(RX_ID + canHandle, 0xFFF, CANStandard, canHandle);
    // }
}

float handlerSetSpeedMotor(Motor *motor, CANMsg *motorSpeedMsg) {
    if (directMotorControlEnabled) {
        double setSpeed = 0.0;
        *motorSpeedMsg >> setSpeed;
        setSpeed /= 10.0;
        motor->speed(setSpeed);

        return setSpeed;
    }
    // if (directMotorControlEnabled) {
    //     uint32_t rawData = 0;
    //     *motorSpeedMsg >> rawData;
    //     rawData = __REV(rawData);

    //     double *setSpeed = reinterpret_cast <double*>(rawData);

    //     motor->speed(*setSpeed);

    //     return setSpeed;
    // }
    else {
        ledErr = 1;
        pc.printf("ERROR: Direct motor control disabled\r\n");

        return 0.0;
    }
}

// float handlerSetVelocityJoint(int joint, CANMsg velocityMsg) {
//     if (velocityPIDControlEnabled) {
//         double setVelocity = 0.0;
//         velocityMsg >> setVelocity;
//         // TODO

//         return setVelocity;
//     }
//     else {
//         ledErr = 1;
//         pc.printf("ERROR: Velocity PID control disabled\r\n");

//         return 0.0;
//     }
// }

void proccessCANMsg(CANMsg *newMsg) {
    switch (newMsg->id) {
        // case configureDirectMotorControl: 
        //     pc.printf("\r\nRecieved command configureDirectMotorControl\r\n");

        //     *newMsg >> directMotorControlEnabled;

        //     if (directMotorControlEnabled) {
        //         pc.printf("Enabled directMotorControl\r\n");
        //     }
        //     else {
        //         pc.printf("Disabled directMotorControl\r\n");
        //     }

        //     break;

        case setSpeedMotor1:
            pc.printf("\r\nRecieved command setSpeedMotor1\r\n");
            pc.printf("Set the speed of motor 1 to %f\r\n", handlerSetSpeedMotor(&motor1, newMsg));

            break;

        case setSpeedMotor2:
            pc.printf("\r\nRecieved command setSpeedMotor2\r\n");
            pc.printf("Set the speed of motor 2 to %f\r\n", handlerSetSpeedMotor(&motor2, newMsg));

            break;

        case setSpeedMotor3:
            pc.printf("\r\nRecieved command setSpeedMotor3\r\n");
            pc.printf("Set the speed of motor 3 to %f\r\n", handlerSetSpeedMotor(&motor3, newMsg));

            break;

        // case configureVelocityPIDControl:
        //     pc.printf("\r\nRecieved command configureVelocityPIDControl\r\n");

        //     *newMsg >> velocityPIDControlEnabled;

        //     if (velocityPIDControlEnabled) {
        //         pc.printf("Enabled velocity PID control\r\n");
        //     }
        //     else {
        //         pc.printf("Disabled velocity PID control\r\n");
        //     }

        //     break;

        // case setVelocityJoint1:
        //     pc.printf("\r\nRecieved command setVelocityJoint1\r\n");
        //     pc.printf("Set the velocity of joint 1 to %f\r\n", handlerSetVelocityJoint(1, newMsg));

        //     break;

        // case setVelocityJoint2:
        //     pc.printf("\r\nRecieved command setVelocityJoint2\r\n");
        //     pc.printf("Set the velocity of joint 2 to %f\r\n", handlerSetVelocityJoint(2, newMsg));

        //     break;

        // case setVelocityJoint3:
        //     pc.printf("\r\nRecieved command setVelocityJoint3\r\n");
        //     pc.printf("Set the velocity of joint 3 to %f\r\n", handlerSetVelocityJoint(3, newMsg));

        //     break;

        default:
            pc.printf("Recieved unimplemented command\r\n");

            break;
    }
}

// void updateJointAngles() { 
//     jointAngle[0] = absEnc1.avgDutyCycle() * 360.0; 
//     jointAngle[1] = absEnc2.avgDutyCycle() * 360.0; 
//     jointAngle[2] = absEnc3.avgDutyCycle() * 360.0; 
// } 
 
// void sendJointAnglesToJetson() { 

//     for (int i = 0; i < 3; i++) { 
//         double a = jointAngle[i];
//         char arr[sizeof(a)];
//         memcpy(arr,&a,sizeof(a));

//         CANMsg txMsg(TX_ID, arr, sizeof(arr));
         
//         if(can.write(txMsg)) { 
//             // pc.printf("Sent joint %d angle to jetson\r\n", i); 
//         } 
//         else {
//             pc.printf("ERROR: CAN send error!\r\n");
//         }
//     } 
// } 
 
int main(void)
{
    pc.printf("Program Started\r\n\r\n");

    initCAN();

    canSendTimer.start();

    while (1) {

        if (can.read(rxMsg)) {
            proccessCANMsg(&rxMsg);
            rxMsg.clear();
            ledCAN = !ledCAN;
        }

        // updateJointAngles();

        // if (canSendTimer.read() > 0.1) {
        //     sendJointAnglesToJetson();
        //     canSendTimer.reset();
        // }

    }
}
 
