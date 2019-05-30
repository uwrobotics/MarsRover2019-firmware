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

#include <algorithm>
#include <MaximInterface/Links/OneWireMaster.hpp>
#include <MaximInterface/Utilities/Error.hpp>
#include "DS2431.hpp"

namespace MaximInterface {

error_code writeMemory(DS2431 & device, DS2431::Address targetAddress,
                       DS2431::Scratchpad::const_span data) {
  error_code result = device.writeScratchpad(targetAddress, data);
  if (result) {
    return result;
  }
  DS2431::Scratchpad::array readData;
  uint_least8_t esByte;
  result = device.readScratchpad(readData, esByte);
  if (result) {
    return result;
  }
  result = device.copyScratchpad(targetAddress, esByte);
  return result;
}

error_code DS2431::readMemory(Address beginAddress,
                              span<uint_least8_t> data) const {  
  error_code result = selectRom(*master);
  if (result) {
    return result;
  }
  const uint_least8_t sendBlock[] = {0xF0, beginAddress, 0x00};
  result = master->writeBlock(sendBlock);
  if (result) {
    return result;
  }
  result = master->readBlock(data);
  return result;
}

error_code DS2431::writeScratchpad(Address targetAddress,
                                   Scratchpad::const_span data) {  
  error_code result = selectRom(*master);
  if (result) {
    return result;
  }
  uint_least8_t block[3 + Scratchpad::size] = {0x0F, targetAddress, 0x00};
  std::copy(data.begin(), data.end(), block + 3);
  result = master->writeBlock(block);
  if (result) {
    return result;
  }
  const uint_fast16_t calculatedCrc = calculateCrc16(block) ^ 0xFFFFU;
  result = master->readBlock(make_span(block, 2));
  if (result) {
    return result;
  }
  if (calculatedCrc !=
      ((static_cast<uint_fast16_t>(block[1]) << 8) | block[0])) {
    result = make_error_code(CrcError);
  }
  return result;
}

error_code DS2431::readScratchpad(Scratchpad::span data,
                                  uint_least8_t & esByte) {
  typedef array<uint_least8_t, 6 + Scratchpad::size> Block;

  error_code result = selectRom(*master);
  if (result) {
    return result;
  }
  Block block = {0xAA};
  result = master->writeByte(block.front());
  if (result) {
    return result;
  }
  result = master->readBlock(make_span(block).subspan(1));
  if (result) {
    return result;
  }
  Block::const_iterator blockIt = block.end();
  uint_fast16_t receivedCrc = static_cast<uint_fast16_t>(*(--blockIt)) << 8;
  receivedCrc |= *(--blockIt);
  const uint_fast16_t expectedCrc =
      calculateCrc16(make_span(block.data(), block.size() - 2)) ^ 0xFFFFU;
  if (expectedCrc == receivedCrc) {
    Block::const_iterator blockItEnd = blockIt;
    blockIt -= data.size();
    std::copy(blockIt, blockItEnd, data.begin());
    esByte = *(--blockIt);
  } else {
    result = make_error_code(CrcError);
  }
  return result;
}

error_code DS2431::copyScratchpad(Address targetAddress, uint_least8_t esByte) {  
  error_code result = selectRom(*master);
  if (result) {
    return result;
  }
  uint_least8_t block[] = {0x55, targetAddress, 0x00};
  result = master->writeBlock(block);
  if (result) {
    return result;
  }
  result = master->writeByteSetLevel(esByte, OneWireMaster::StrongLevel);
  if (result) {
    return result;
  }
  sleep->invoke(10);
  result = master->setLevel(OneWireMaster::NormalLevel);
  if (result) {
    return result;
  }
  result = master->readByte(block[0]);
  if (result) {
    return result;
  }
  if (block[0] != 0xAA) {
    result = make_error_code(OperationFailure);
  }
  return result;
}

const error_category & DS2431::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "DS2431"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case CrcError:
        return "CRC Error";

      case OperationFailure:
        return "Operation Failure";        
      }
      return defaultErrorMessage(condition);
    }
  } instance;
  return instance;
}

} // namespace MaximInterface
