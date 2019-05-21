#ifndef ROVER_CONFIG_H
#define ROVER_CONFIG_H

// Serial config
#define ROVER_DEFAULT_BAUD_RATE 115200

// CAN bus config
#define ROVER_CANBUS_FREQUENCY                  500000 // 500 kbps
#define ROVER_CANID_FILTER_MASK                 0xF00  // Use bits 8:11 for addressing and 0:7 for command/message type
#define ROVER_SAFETY_CANID	                    0x100
#define ROVER_SCIENCE_CANID	                    0x200
#define ROVER_ARM_LOWER_CANID                   0x300
#define ROVER_ARM_UPPER_CANID                   0x400
#define ROVER_JETSON_CANID                      0x500
#define ROVER_JETSON_START_CANID_MSG_ARM_LOWER  0x500
#define ROVER_JETSON_START_CANID_MSG_ARM_UPPER  0x503
#define ROVER_JETSON_START_CANID_MSG_SCIENCE    0x510
#define ROVER_JETSON_START_CANID_MSG_SAFETY     0x530

// Controls
#define ROVER_MOTOR_PWM_FREQ_HZ 1000    // 1 kHz

#endif // ROVER_CONFIG_H