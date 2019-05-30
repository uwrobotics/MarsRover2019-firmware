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

#ifndef MaximInterface_GpioOneWireMaster
#define MaximInterface_GpioOneWireMaster

#include <MaximInterface/Links/OneWireMaster.hpp>
#include <DigitalOut.h>

namespace MaximInterface {

class GpioOneWireMaster : public OneWireMaster {
public:
  /// @param owGpio Pin to use for 1-Wire bus
  /// @param extSpu Pin to use for external Strong Pullup
  explicit GpioOneWireMaster(PinName owGpio, PinName extSpu = NC,
                             bool extSpuActiveHigh = false);

  error_code initialize();

  virtual error_code reset();
  virtual error_code touchBitSetLevel(bool & sendRecvBit, Level afterLevel);
  virtual error_code setSpeed(Speed newSpeed);
  virtual error_code setLevel(Level newLevel);

private:
  unsigned int port;
  unsigned int pin;
  Speed speed;
  mbed::DigitalOut extSpu;
  bool extSpuActiveHigh;

  inline void writeGpioLow();
  inline void writeGpioHigh();
  inline bool readGpio();
  inline void setGpioMode(unsigned int mode);
};

} // namespace MaximInterface

#endif
