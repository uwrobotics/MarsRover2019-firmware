/*
 * mbed SDK
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Automatically generated configuration file.
// DO NOT EDIT, content will be overwritten.

#ifndef __MBED_CONFIG_DATA__
#define __MBED_CONFIG_DATA__

#include "mbed_assert.h"

// Macros
#define MBED_ASSERT_SUCCESS_RETURN_ERROR(functionCall) {    \
    mbed_error_status_t result = functionCall;              \
    MBED_ASSERT(result == MBED_SUCCESS);                    \
    if (result != MBED_SUCCESS) {                           \
        return result;                                      \
    }                                                       \
}

#define MBED_ASSERT_SUCCESS(functionCall) MBED_ASSERT(functionCall == MBED_SUCCESS)

// Configuration parameters
#define CLOCK_SOURCE                                        USE_PLL_HSE_EXTC|USE_PLL_HSI            // set by target:NUCLEO_F091RC
#define LPTICKER_DELAY_TICKS                                1                                       // set by target:NUCLEO_F091RC
#define MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE         115200                                  // set by Alex
#define MBED_CONF_PLATFORM_ERROR_ALL_THREADS_INFO           0                                       // set by library:platform
#define MBED_CONF_PLATFORM_ERROR_FILENAME_CAPTURE_ENABLED   0                                       // set by library:platform
#define MBED_CONF_PLATFORM_ERROR_HIST_ENABLED               0                                       // set by library:platform
#define MBED_CONF_PLATFORM_ERROR_HIST_SIZE                  4                                       // set by library:platform
#define MBED_CONF_PLATFORM_FORCE_NON_COPYABLE_ERROR         0                                       // set by library:platform
#define MBED_CONF_PLATFORM_MAX_ERROR_FILENAME_LEN           16                                      // set by library:platform
#define MBED_CONF_PLATFORM_POLL_USE_LOWPOWER_TIMER          0                                       // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_BAUD_RATE                  115200                                  // set by Alex
#define MBED_CONF_PLATFORM_STDIO_BUFFERED_SERIAL            0                                       // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_CONVERT_NEWLINES           0                                       // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_CONVERT_TTY_NEWLINES       0                                       // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_FLUSH_AT_EXIT              1                                       // set by library:platform
#define MBED_CONF_TARGET_LPUART_CLOCK_SOURCE                USE_LPUART_CLK_LSE|USE_LPUART_CLK_PCLK1 // set by target:FAMILY_STM32
#define MBED_CONF_TARGET_LSE_AVAILABLE                      1                                       // set by target:FAMILY_STM32

#define __CORTEX_M0
#define CMSIS_VECTAB_VIRTUAL
#define ARM_MATH_CM0
#define TOOLCHAIN_GCC
#define TOOLCHAIN_GCC_ARM

#define TARGET_STM32F0
#define TARGET_CORTEX_M
#define TARGET_NUCLEO_F091RC
#define TARGET_STM
#define TARGET_STM32F091RC

#define CMSIS_VECTAB_VIRTUAL_HEADER_FILE                    "cmsis_nvic.h"

#define DEVICE_I2CSLAVE                                     1
#define DEVICE_PORTOUT                                      1
#define DEVICE_PORTINOUT                                    1
#define DEVICE_RTC                                          1
#define DEVICE_SERIAL_ASYNCH                                1
#define DEVICE_CAN                                          1
#define DEVICE_I2C_ASYNCH                                   1
#define DEVICE_ANALOGOUT                                    1
#define DEVICE_SPI_ASYNCH                                   1
#define DEVICE_LPTICKER                                     1
#define DEVICE_PWMOUT                                       1
#define DEVICE_INTERRUPTIN                                  1
#define DEVICE_I2C                                          1
#define DEVICE_STDIO_MESSAGES                               1
#define DEVICE_SERIAL_FC                                    1
#define DEVICE_PORTIN                                       1
#define DEVICE_SLEEP                                        1
#define DEVICE_CRC                                          1
#define DEVICE_SPI                                          1
#define DEVICE_SPISLAVE                                     1
#define DEVICE_ANALOGIN                                     1
#define DEVICE_SERIAL                                       1
#define DEVICE_FLASH                                        1

#define TRANSACTION_QUEUE_SIZE_SPI                          2

#endif
