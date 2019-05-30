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

#include "DS9400.hpp"

namespace MaximInterface {

error_code DS9400::waitAwake() {
  error_code result;
  uint_least8_t data;
  do {
    result = uart->readByte(data);
  } while (!result && data != 0xA5);
  return result;
}

error_code DS9400::start() { return uart->writeByte('S'); }

error_code DS9400::start(uint_least8_t address) {
  error_code result = start();
  if (!result) {
    result = writeByte(address);
  }
  return result;
}

error_code DS9400::stop() { return uart->writeByte('P'); }

error_code DS9400::writeByte(uint_least8_t data) {
  const uint_least8_t packet[] = {'Q', data};
  error_code result = uart->clearReadBuffer();
  if (!result) {
    result = uart->writeBlock(packet);
    if (!result) {
      result = uart->readByte(data);
      if (!result && data != 0) {
        result = make_error_code(I2CMaster::NackError);
      }
    }
  }
  return result;
}

error_code DS9400::readByte(AckStatus status, uint_least8_t & data) {
  error_code result = uart->clearReadBuffer();
  if (!result) {
    result = uart->writeByte(status == Ack ? 'R' : 'N');
    if (!result) {
      result = uart->readByte(data);
    }
  }
  return result;
}

error_code DS9400::configure(uint_least8_t config) {
  const uint_least8_t packet[] = {'C', config};
  return uart->writeBlock(packet);
}

} // namespace MaximInterface
