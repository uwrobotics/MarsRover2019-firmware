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

#ifndef MaximInterface_mbed_Uart
#define MaximInterface_mbed_Uart

#include <Serial.h>
#include <MaximInterface/Links/Uart.hpp>

namespace MaximInterface {
namespace mbed {

/// Wrapper for mbed::Serial.
class Uart : public MaximInterface::Uart {
public:
  enum ErrorValue {
    HardwareError = 1 ///< Write operation aborted due to timeout.
  };

  explicit Uart(::mbed::Serial & serial) : serial(&serial) {}

  void setSerial(::mbed::Serial & serial) { this->serial = &serial; }

  virtual error_code setBaudRate(int_least32_t baudRate);
  virtual error_code sendBreak();
  virtual error_code clearReadBuffer();
  virtual error_code writeByte(uint_least8_t data);
  virtual error_code readByte(uint_least8_t & data);

  static const error_category & errorCategory();

private:
  ::mbed::Serial * serial;
};

inline error_code make_error_code(Uart::ErrorValue e) {
  return error_code(e, Uart::errorCategory());
}

} // namespace mbed
} // namespace MaximInterface

#endif
