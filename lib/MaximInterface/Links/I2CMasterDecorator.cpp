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

#include "I2CMasterDecorator.hpp"

namespace MaximInterface {

error_code I2CMasterDecorator::start(uint_least8_t address) {
  return master->start(address);
}

error_code I2CMasterDecorator::stop() { return master->stop(); }

error_code I2CMasterDecorator::writeByte(uint_least8_t data) {
  return master->writeByte(data);
}

error_code I2CMasterDecorator::writeBlock(span<const uint_least8_t> data) {
  return master->writeBlock(data);
}

error_code I2CMasterDecorator::writePacketImpl(uint_least8_t address,
                                               span<const uint_least8_t> data,
                                               bool sendStop) {
  return master->writePacket(address, data, sendStop);
}

error_code I2CMasterDecorator::readByte(AckStatus status,
                                        uint_least8_t & data) {
  return master->readByte(status, data);
}

error_code I2CMasterDecorator::readBlock(AckStatus status,
                                         span<uint_least8_t> data) {
  return master->readBlock(status, data);
}

error_code I2CMasterDecorator::readPacketImpl(uint_least8_t address,
                                              span<uint_least8_t> data,
                                              bool sendStop) {
  return master->readPacket(address, data, sendStop);
}

} // namespace MaximInterface
