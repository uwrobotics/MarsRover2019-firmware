/*******************************************************************************
* Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************/

#ifndef MaximInterface_owlink
#define MaximInterface_owlink

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Timing parameters for the 1-Wire bus in microseconds.
struct OneWireTiming {
  uint16_t tRSTL; ///< Reset Low Time
  uint16_t tMSP;  ///< Presence-Detect Sample Time
  uint16_t tW0L;  ///< Write-Zero Low Time
  uint16_t tW1L;  ///< Write-One Low Time
  uint16_t tMSR;  ///< Read Sample Time
  uint16_t tSLOT; ///< Time Slot Duration
};

/// Delay for the specified number of microseconds.
void ow_usdelay(unsigned int time_us);

/// @brief Send and receive one bit, and set a new level on the 1-Wire bus.
/// @note GPIO pin must be configured for open drain operation.
/// @param[in,out] sendRecvBit
/// Buffer containing the bit to send on 1-Wire bus in lsb.
/// Read data from 1-Wire bus will be returned in lsb.
/// @param[in] inReg Input register for GPIO pin.
/// @param[in,out] outReg Output register for GPIO pin.
/// @param pinMask Pin mask for input and output registers.
/// @param[in] timing 1-Wire timing parameters to use.
void ow_bit(uint8_t * sendRecvBit, const volatile uint32_t * inReg,
            volatile uint32_t * outReg, unsigned int pinMask,
            const OneWireTiming * timing);

#ifdef __cplusplus
}
#endif

#endif /* MaximInterface_owlink */
