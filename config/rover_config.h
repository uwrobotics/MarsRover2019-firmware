#ifndef ROVER_CONFIG_H
#define ROVER_CONFIG_H

// Serial config
#define ROVER_DEFAULT_BAUD_RATE 115200

// CAN bus config
#define ROVER_CANBUS_FREQUENCY	1000000 // 1 Mbps
#define ROVER_CANID_FILTER_MASK 0xF00   // Use bits 8:11 for addressing and 0:7 for command/message type
#define ROVER_SAFETY_CANID		0x100
#define ROVER_SCIENCE_CANID		0x200
#define ROVER_ARMO_CANID		0x300
#define ROVER_ARM1_CANID		0x400
#define ROVER_JETSON_CANID   	0x500

// Controls
#define ROVER_MOTOR_PWM_FREQ_HZ 1000    // 1 kHz

#endif // ROVER_CONFIG_H