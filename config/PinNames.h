/* mbed Microcontroller Library
 *******************************************************************************
 * Copyright (c) 2018, STMicroelectronics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 */

#ifndef MBED_PINNAMES_H
#define MBED_PINNAMES_H

#include "cmsis.h"
#include "PinNamesTypes.h"
#include "mbed_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// GPIO definitions
#define CAN_GPIO_RX_PIN     GPIO_PIN_8
#define CAN_GPIO_TX_PIN     GPIO_PIN_9
#define CAN_GPIO_PORT       GPIOB

typedef enum {
    ALT0  = 0x100,
    ALT1  = 0x200,
    ALT2  = 0x300,
    ALT3  = 0x400
} ALTx;

typedef enum {
    PA_0  = 0x00,
    PA_1  = 0x01,
    PA_2  = 0x02,
    PA_3  = 0x03,
    PA_4  = 0x04,
    PA_5  = 0x05,
    PA_6  = 0x06,
    PA_6_ALT0 = PA_6 | ALT0,
    PA_7  = 0x07,
    PA_7_ALT0 = PA_7 | ALT0,
    PA_7_ALT1 = PA_7 | ALT1,
    PA_7_ALT2 = PA_7 | ALT2,
    PA_8  = 0x08,
    PA_9  = 0x09,
    PA_10 = 0x0A,
    PA_11 = 0x0B,
    PA_12 = 0x0C,
    PA_13 = 0x0D,
    PA_14 = 0x0E,
    PA_15 = 0x0F,
    PB_0  = 0x10,
    PB_0_ALT0 = PB_0 | ALT0,
    PB_1  = 0x11,
    PB_1_ALT0 = PB_1 | ALT0,
    PB_1_ALT1 = PB_1 | ALT1,
    PB_2  = 0x12,
    PB_3  = 0x13,
    PB_4  = 0x14,
    PB_5  = 0x15,
    PB_6 = 0x16,
    PB_7 = 0x17,
    PB_8 = 0x18,
    PB_9 = 0x19,
    PB_10 = 0x1A,
    PB_11 = 0x1B,
    PB_12 = 0x1C,
    PB_13 = 0x1D,
    PB_14 = 0x1E,
    PB_14_ALT0 = PB_14 | ALT0,
    PB_15 = 0x1F,
    PB_15_ALT0 = PB_15 | ALT0,
    PB_15_ALT1 = PB_15 | ALT1,

    PC_0 = 0x20,
    PC_0_ALT0 = PC_0 | ALT0,
    PC_1 = 0x21,
    PC_1_ALT0 = PC_1 | ALT0,
    PC_2 = 0x22,
    PC_3 = 0x23,
    PC_4 = 0x24,
    PC_5 = 0x25,
    PC_6 = 0x26,
    PC_7 = 0x27,
    PC_8 = 0x28,
    PC_9 = 0x29,
    PC_10 = 0x2A,
    PC_10_ALT0 = PC_10 | ALT0,
    PC_11 = 0x2B,
    PC_11_ALT0 = PC_11 | ALT0,
    PC_12 = 0x2C,
    PC_13 = 0x2D,
    PC_14 = 0x2E,
    PC_15 = 0x2F,

    PD_2 = 0x32,

    PF_0 = 0x50,
    PF_1 = 0x51,
    PF_11 = 0x5B,

    // ADC internal channels
    ADC_TEMP = 0xF0,
    ADC_VREF = 0xF1,
    ADC_VBAT = 0xF2,

#ifdef ROVERBOARD_ARM_PINMAP        // ARM BOARD PIN MAPPINGS BEGIN
#define ROVERBOARD_COMMON_PINMAP

    // I2C
    I2C_SCL = PB_6,
    I2C_SDA = PB_7,

    // PWM Encoders
    ENC_A3 = PC_9,
    ENC_A2 = PA_9,
    ENC_A1 = PA_10,

    // Quadrature Encoders
    ENCR1_CH1 = PA_7,
    ENCR1_CH2 = PC_4,
    ENCR1_INDEX = PC_5,

    // PWM Output
    MOTOR1 = PB_13,
    MOTOR1_DIR = PB_14,
    MOTOR2 = PB_15,
    MOTOR2_DIR = PC_6,
    MOTOR3 = PC_7,
    MOTOR3_DIR = PC_8,

    // Buttons
    BUTTON_1 = PA_0,
    BUTTON_2 = PA_1,

    // Limit switches
    LIM_1A = PB_11,
    LIM_1B = PB_12,
    LIM_2A = PB_3,
    LIM_2B = PB_4,
    LIM_3A = PA_11,
    LIM_3B = PA_12,
    
    // TODO: Add remaining pins
    
#endif                              // ARM BOARD PIN MAPPINGS END


#ifdef ROVERBOARD_SCIENCE_PINMAP    // SCIENCE BOARD PIN MAPPINGS BEGIN
#define ROVERBOARD_COMMON_PINMAP

    // PWM Output
    MOTOR_A = PB_15,		// Auger
    MOTOR_A_DIR = PC_6,
    MOTOR_C = PC_7,			// Centrifuge
    MOTOR_C_DIR = PC_8,
    MOTOR_E = PB_13,		// Elevator
    MOTOR_E_DIR = PB_14,
	SERVO_F = PA_9,			// Funnel
	SERVO_P = PC_9,			// Probe

    // Elevator encoders
    ENC_E_CH1 = PB_0,
    ENC_E_CH2 = PB_1,
    ENC_E_INDEX = PB_2,

	// Centrifuge encoder,
	ENC_C_CH1 = PA_7,
	ENC_C_CH2 = PC_4,
	ENC_C_INDEX = PC_5,

    // Limit Switches
    E_LS_T = PA_11,
    E_LS_B = PA_12,
    C_LS = PA_8,

    // Moisture sensor
	MOIST_PWR = PA_4,
	MOIST_DATA = PA_5,

	// Temperature sensor
	TEMP_DATA = PB_12,

	// Ultrasonic sensors,
	ULTRA_TRIG_1 = PB_10,
	ULTRA_ECHO_1 = PB_11,
	ULTRA_TRIG_2 = PA_2,
	ULTRA_ECHO_2 = PA_3,

	// Neopixel signal
	NEO_PIXEL_SIGNAL = PB_5,

#endif                              // SCIENCE BOARD PIN MAPPINGS END

    
#ifdef ROVERBOARD_SAFETY_PINMAP     // SAFETY BOARD PIN MAPPINGS BEGIN
#define ROVERBOARD_COMMON_PINMAP

    // I2C
    I2C_SCL = PB_6,
    I2C_SDA = PB_7,
    
    // TODO: Add remaining pins

#endif                              // SAFETY BOARD PIN MAPPINGS END

    
#ifdef ROVERBOARD_COMMON_PINMAP     // COMMON PIN MAPPINGS FOR ROVER BOARDS BEGIN

    // LEDs
    LED1 = PC_0,
    LED2 = PC_1,
    LED3 = PC_2,
    LED4 = PC_3,

    // CAN
    CAN_RX = PB_8,
    CAN_TX = PB_9,

    // Serial
    SERIAL_TX = PC_10,
    SERIAL_RX = PC_11,
    SERIAL_RTS = PD_2,

    // STDIO for console print
#ifdef MBED_CONF_TARGET_STDIO_UART_TX
    STDIO_UART_TX = MBED_CONF_TARGET_STDIO_UART_TX,
#else
    STDIO_UART_TX = SERIAL_TX,
#endif
#ifdef MBED_CONF_TARGET_STDIO_UART_RX
    STDIO_UART_RX = MBED_CONF_TARGET_STDIO_UART_RX,
#else
    STDIO_UART_RX = SERIAL_TX,
#endif

    // TODO: Add more!

#endif                              // COMMON PIN MAPPINGS FOR ROVER BOARDS END

    
#ifdef NUCLEO_PINMAP                // NUCLEO BOARD PIN MAPPINGS BEGIN
    
    // Arduino connector namings
    A0          = PA_0,
    A1          = PA_1,
    A2          = PA_4,
    A3          = PB_0,
    A4          = PC_1,
    A5          = PC_0,
    D0          = PA_3,
    D1          = PA_2,
    D2          = PA_10,
    D3          = PB_3,
    D4          = PB_5,
    D5          = PB_4,
    D6          = PB_10,
    D7          = PA_8,
    D8          = PA_9,
    D9          = PC_7,
    D10         = PB_6,
    D11         = PA_7,
    D12         = PA_6,
    D13         = PA_5,
    D14         = PB_9,
    D15         = PB_8,

    // STDIO for console print
#ifdef MBED_CONF_TARGET_STDIO_UART_TX
    STDIO_UART_TX = MBED_CONF_TARGET_STDIO_UART_TX,
#else
    STDIO_UART_TX = PA_2,
#endif
#ifdef MBED_CONF_TARGET_STDIO_UART_RX
    STDIO_UART_RX = MBED_CONF_TARGET_STDIO_UART_RX,
#else
    STDIO_UART_RX = PA_3,
#endif

    // Generic signals namings
    LED1        = PA_5,
    LED2        = PA_5,
    LED3        = PA_5,
    LED4        = PA_5,
    USER_BUTTON = PC_13,
    // Standardized button names
    BUTTON1 = USER_BUTTON,
    SERIAL_TX   = STDIO_UART_TX,
    SERIAL_RX   = STDIO_UART_RX,
    USBTX       = STDIO_UART_TX,
    USBRX       = STDIO_UART_RX,
    I2C_SCL     = PB_8,
    I2C_SDA     = PB_9,
    SPI_MOSI    = PA_7,
    SPI_MISO    = PA_6,
    SPI_SCK     = PA_5,
    SPI_CS      = PB_6,
    PWM_OUT     = PB_4,

    /**** OSCILLATOR pins ****/
    RCC_OSC32_IN = PC_14,
    RCC_OSC32_OUT = PC_15,
    RCC_OSC_IN = PF_0,
    RCC_OSC_OUT = PF_1,

    /**** DEBUG pins ****/
    SYS_SWCLK = PA_14,
    SYS_SWDIO = PA_13,
    SYS_WKUP1 = PA_0,
    SYS_WKUP2 = PC_13,
    SYS_WKUP4 = PA_2,
    SYS_WKUP5 = PC_5,
    SYS_WKUP6 = PB_5,
    SYS_WKUP7 = PB_15,

#endif                              // NUCLEO BOARD PIN MAPPINGS END

    // Not connected
    NC = (int)0xFFFFFFFF
    
} PinName;

#ifdef __cplusplus
}
#endif

#endif
