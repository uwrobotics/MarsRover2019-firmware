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

#ifndef MaximInterface_DS2480B
#define MaximInterface_DS2480B

#include <MaximInterface/Links/OneWireMaster.hpp>
#include <MaximInterface/Links/Sleep.hpp>
#include <MaximInterface/Links/Uart.hpp>
#include <MaximInterface/Utilities/Export.h>

namespace MaximInterface {

/// Serial to 1-Wire Line Driver
class DS2480B : public OneWireMaster {
public:
  enum ErrorValue { HardwareError = 1 };

  DS2480B(Sleep & sleep, Uart & uart) : sleep(&sleep), uart(&uart) {}

  void setSleep(Sleep & sleep) { this->sleep = &sleep; }
  void setUart(Uart & uart) { this->uart = &uart; }

  MaximInterface_EXPORT error_code initialize();

  MaximInterface_EXPORT virtual error_code reset();
  
  MaximInterface_EXPORT virtual error_code touchBitSetLevel(bool & sendRecvBit,
                                                            Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  writeByteSetLevel(uint_least8_t sendByte, Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  readByteSetLevel(uint_least8_t & recvByte, Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code setSpeed(Speed newSpeed);
  
  MaximInterface_EXPORT virtual error_code setLevel(Level newLevel);

  MaximInterface_EXPORT static const error_category & errorCategory();

protected:
  MaximInterface_EXPORT error_code sendCommand(uint_least8_t command);

private:
  const Sleep * sleep;
  Uart * uart;

  Level level;
  uint_least8_t mode;
  uint_least8_t speed;
};

inline error_code make_error_code(DS2480B::ErrorValue e) {
  return error_code(e, DS2480B::errorCategory());
}

} // namespace MaximInterface

#endif
