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

#include <MaximInterface/Utilities/Error.hpp>
#include "OneWireMaster.hpp"

namespace MaximInterface {

static const int maxBitNum = 7;

error_code OneWireMaster::writeByteSetLevel(uint_least8_t sendByte,
                                            Level afterLevel) {
  error_code result;
  for (int bitNum = 0; (bitNum <= maxBitNum) && !result; ++bitNum) {
    result = writeBitSetLevel(((sendByte >> bitNum) & 1) == 1,
                              (bitNum == maxBitNum) ? afterLevel : NormalLevel);
  }
  return result;
}

error_code OneWireMaster::readByteSetLevel(uint_least8_t & recvByte,
                                           Level afterLevel) {
  recvByte = 0;
  error_code result;
  for (int bitNum = 0; bitNum <= maxBitNum; ++bitNum) {
    bool recvBit;
    result = readBitSetLevel(recvBit,
                             (bitNum == maxBitNum) ? afterLevel : NormalLevel);
    if (result) {
      break;
    }
    if (recvBit) {
      recvByte |= (1 << bitNum);
    }
  }
  return result;
}

error_code OneWireMaster::writeBlock(span<const uint_least8_t> sendBuf) {
  error_code result;
  for (span<const uint_least8_t>::index_type i = 0;
       i < sendBuf.size() && !result; ++i) {
    result = writeByte(sendBuf[i]);
  }
  return result;
}

error_code OneWireMaster::readBlock(span<uint_least8_t> recvBuf) {
  error_code result;
  for (span<uint_least8_t>::index_type i = 0; i < recvBuf.size() && !result;
       ++i) {
    result = readByte(recvBuf[i]);
  }
  return result;
}

error_code OneWireMaster::triplet(TripletData & data) {
  error_code result = readBit(data.readBit);
  if (!result) {
    result = readBit(data.readBitComplement);
  }
  if (!result) {
    if (data.readBit) {
      data.writeBit = 1;
    } else if (data.readBitComplement) {
      data.writeBit = 0;
    }
    // else: use data.writeBit parameter
    result = writeBit(data.writeBit);
  }
  return result;
}

const error_category & OneWireMaster::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "OneWireMaster"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case NoSlaveError:
        return "No Slave Error";

      case ShortDetectedError:
        return "Short Detected Error";

      case InvalidSpeedError:
        return "Invalid Speed Error";

      case InvalidLevelError:
        return "Invalid Level Error";

      default:
        return defaultErrorMessage(condition);
      }
    }
  } instance;
  return instance;
}

} // namespace MaximInterface
