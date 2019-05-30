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

#include "OneWireMasterDecorator.hpp"

namespace MaximInterface {

error_code OneWireMasterDecorator::reset() { return master->reset(); }

error_code OneWireMasterDecorator::touchBitSetLevel(bool & sendRecvBit,
                                                    Level afterLevel) {
  return master->touchBitSetLevel(sendRecvBit, afterLevel);
}

error_code OneWireMasterDecorator::writeByteSetLevel(uint_least8_t sendByte,
                                                     Level afterLevel) {
  return master->writeByteSetLevel(sendByte, afterLevel);
}

error_code OneWireMasterDecorator::readByteSetLevel(uint_least8_t & recvByte,
                                                    Level afterLevel) {
  return master->readByteSetLevel(recvByte, afterLevel);
}

error_code
OneWireMasterDecorator::writeBlock(span<const uint_least8_t> sendBuf) {
  return master->writeBlock(sendBuf);
}

error_code OneWireMasterDecorator::readBlock(span<uint_least8_t> recvBuf) {
  return master->readBlock(recvBuf);
}

error_code OneWireMasterDecorator::setSpeed(Speed newSpeed) {
  return master->setSpeed(newSpeed);
}

error_code OneWireMasterDecorator::setLevel(Level newLevel) {
  return master->setLevel(newLevel);
}

error_code OneWireMasterDecorator::triplet(TripletData & data) {
  return master->triplet(data);
}

} // namespace MaximInterface
