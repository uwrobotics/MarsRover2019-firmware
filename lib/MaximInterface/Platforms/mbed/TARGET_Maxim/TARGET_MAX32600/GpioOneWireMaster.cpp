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

#ifdef TARGET_MAX32600

#include "GpioOneWireMaster.hpp"
#include "owlink.h"
#include <gpio_regs.h>
#include <clkman_regs.h>

namespace MaximInterface {

static const OneWireTiming stdTiming = {
    560, // tRSTL
    68,  // tMSP
    64,  // tW0L
    8,   // tW1L
    12,  // tMSR
    70,  // tSLOT
};

static const OneWireTiming odTiming = {
    56, // tRSTL
    8,  // tMSP
    8,  // tW0L
    1,  // tW1L
    1,  // tMSR
    10, // tSLOT
};

GpioOneWireMaster::GpioOneWireMaster(PinName owGpio, PinName extSpu,
                                     bool extSpuActiveHigh)
    : port(PINNAME_TO_PORT(owGpio)), pin(PINNAME_TO_PIN(owGpio)),
      speed(StandardSpeed), extSpu(extSpu), extSpuActiveHigh(extSpuActiveHigh) {
}

error_code GpioOneWireMaster::initialize() {
  if (MXC_CLKMAN->clk_ctrl_1_gpio == MXC_E_CLKMAN_CLK_SCALE_DISABLED) {
    MXC_CLKMAN->clk_ctrl_1_gpio = MXC_E_CLKMAN_CLK_SCALE_ENABLED;
  }

  /* Set function */
  MXC_GPIO->func_sel[port] &= ~(0xF << (4 * pin));

  /* Normal input is always enabled */
  MXC_GPIO->in_mode[port] &= ~(0xF << (4 * pin));

  writeGpioHigh();
  setLevel(NormalLevel);

  return error_code();
}

error_code GpioOneWireMaster::reset() {
  const OneWireTiming & curTiming(speed == OverdriveSpeed ? odTiming
                                                          : stdTiming);
  const uint16_t tREC = curTiming.tRSTL - curTiming.tMSP; // tSLOT = 2 *tRSTL

  __disable_irq(); // Enter critical section

  writeGpioLow();                   // Pull low
  ow_usdelay(curTiming.tRSTL);      // Wait specified time for reset pulse
  writeGpioHigh();                  // Let go of pin
  ow_usdelay(curTiming.tMSP);       // Wait specified time for master sample
  const bool pd_pulse = readGpio(); // Get sample
  ow_usdelay(tREC); // Wait for slot time to finish including recovery

  __enable_irq(); // Exit critical section

  return pd_pulse ? make_error_code(NoSlaveError) : error_code();
}

error_code GpioOneWireMaster::touchBitSetLevel(bool & sendRecvBit,
                                               Level afterLevel) {
  __disable_irq(); // Enter critical section

  uint8_t sendRecvUint = sendRecvBit;
  ow_bit(&sendRecvUint, &MXC_GPIO->in_val[port], &MXC_GPIO->out_val[port],
         (1 << pin), ((speed == OverdriveSpeed) ? &odTiming : &stdTiming));
  setLevel(afterLevel);
  sendRecvBit = sendRecvUint;

  __enable_irq(); // Exit critical section

  return error_code();
}

error_code GpioOneWireMaster::setSpeed(Speed newSpeed) {
  if (!((newSpeed == StandardSpeed) || (newSpeed == OverdriveSpeed))) {
    return make_error_code(InvalidSpeedError);
  }
  speed = newSpeed;
  return error_code();
}

error_code GpioOneWireMaster::setLevel(Level newLevel) {
  error_code result;
  switch (newLevel) {
  case NormalLevel:
    setGpioMode(MXC_V_GPIO_OUT_MODE_OPEN_DRAIN);
    if (extSpu.is_connected()) {
      extSpu = !extSpuActiveHigh;
    }
    break;

  case StrongLevel:
    if (extSpu.is_connected()) {
      extSpu = extSpuActiveHigh;
    } else {
      setGpioMode(MXC_V_GPIO_OUT_MODE_NORMAL_DRIVE);
    }
    break;

  default:
    result = make_error_code(InvalidLevelError);
    break;
  }
  return result;
}

inline void GpioOneWireMaster::writeGpioLow() {
  MXC_GPIO->out_val[port] &= ~(1 << pin);
}

inline void GpioOneWireMaster::writeGpioHigh() {
  MXC_GPIO->out_val[port] |= (1 << pin);
}

inline bool GpioOneWireMaster::readGpio() {
  return ((MXC_GPIO->in_val[port] & (1 << pin)) >> pin);
}

inline void GpioOneWireMaster::setGpioMode(unsigned int mode) {
  //read port out_mode
  uint32_t ow_out_mode = MXC_GPIO->out_mode[port];
  //clear the mode for ow_pin
  ow_out_mode &= ~(0xF << (pin * 4));
  //write ow_pin mode and original data back
  MXC_GPIO->out_mode[port] = (ow_out_mode | (mode << (pin * 4)));
}

} // namespace MaximInterface

#endif /* TARGET_MAX32600 */
