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

#ifndef MaximInterface_mbed_I2CMaster
#define MaximInterface_mbed_I2CMaster

#include <I2C.h>
#include <MaximInterface/Links/I2CMaster.hpp>

namespace MaximInterface {
namespace mbed {

/// Wrapper for mbed::I2C.
class I2CMaster : public MaximInterface::I2CMaster {
public:
  explicit I2CMaster(::mbed::I2C & i2c) : i2c(&i2c) {}

  void setI2C(::mbed::I2C & i2c) { this->i2c = &i2c; }

  virtual error_code start(uint_least8_t address);
  virtual error_code stop();
  virtual error_code writeByte(uint_least8_t data);
  virtual error_code readByte(AckStatus status, uint_least8_t & data);

protected:
  virtual error_code readPacketImpl(uint_least8_t address,
                                    span<uint_least8_t> data, bool sendStop);
                                    
  virtual error_code writePacketImpl(uint_least8_t address,
                                     span<const uint_least8_t> data,
                                     bool sendStop);

private:
  ::mbed::I2C * i2c;
};

} // namespace mbed
} // namespace MaximInterface

#endif
