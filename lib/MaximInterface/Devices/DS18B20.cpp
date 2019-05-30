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

#include <MaximInterface/Links/OneWireMaster.hpp>
#include <MaximInterface/Utilities/Error.hpp>
#include "DS18B20.hpp"

namespace MaximInterface {

const uint_least8_t DS18B20::nineBitResolution;
const uint_least8_t DS18B20::tenBitResolution;
const uint_least8_t DS18B20::elevenBitResolution;
const uint_least8_t DS18B20::twelveBitResolution;

error_code DS18B20::initialize() {
  Scratchpad::array scratchpad;
  return readScratchpad(scratchpad);
}

error_code DS18B20::writeScratchpad(uint_least8_t th, uint_least8_t tl,
                                    uint_least8_t res) {  
  error_code result = selectRom(*master);
  if (!result) {
    const uint_least8_t sendBlock[] = {0x4E, th, tl, res};
    result = master->writeBlock(sendBlock);
    if (!result) {
      resolution = res;
    }
  }
  return result;
}

error_code DS18B20::readScratchpad(Scratchpad::span scratchpad) {  
  error_code result = selectRom(*master);
  if (result) {
    return result;
  }
  result = master->writeByte(0xBE);
  if (result) {
    return result;
  }
  result = master->readBlock(scratchpad);
  if (result) {
    return result;
  }
  uint_least8_t receivedCrc;
  result = master->readByte(receivedCrc);
  if (result) {
    return result;
  }
  if (receivedCrc == calculateCrc8(scratchpad)) {
    resolution = scratchpad[4];
  } else {
    result = make_error_code(CrcError);
  }
  return result;
}

error_code DS18B20::readPowerSupply(bool & localPower) {  
  error_code result = selectRom(*master);
  if (result) {
    return result;
  }
  result = master->writeByte(0xB4);
  if (result) {
    return result;
  }
  result = master->touchBit(localPower);
  return result;
}

error_code DS18B20::copyScratchpad() {
  const uint_least8_t copyScratchpadCmd = 0x48;
  
  bool hasLocalPower;
  error_code result = readPowerSupply(hasLocalPower);
  if (result) {
    return result;
  }
  result = selectRom(*master);
  if (result) {
    return result;
  }
  if (hasLocalPower) {
    result = master->writeByte(copyScratchpadCmd);
    bool recvbit = 0;
    while (!recvbit && !result) {
      result = master->touchBit(recvbit);
    }
  } else {
    result = master->writeByteSetLevel(copyScratchpadCmd,
                                       OneWireMaster::StrongLevel);
    if (!result) {
      sleep->invoke(10);
      result = master->setLevel(OneWireMaster::NormalLevel);
    }
  }
  return result;
}

error_code DS18B20::convertTemperature() {
  const uint_least8_t convertTemperatureCmd = 0x44;
  
  bool hasLocalPower;
  error_code result = readPowerSupply(hasLocalPower);
  if (result) {
    return result;
  }
  result = selectRom(*master);
  if (result) {
    return result;
  }
  if (hasLocalPower) {
    result = master->writeByte(convertTemperatureCmd);
    bool recvbit = 0;
    while (!result && !recvbit) {
      result = master->touchBit(recvbit);
    }
  } else {
    result = master->writeByteSetLevel(convertTemperatureCmd,
                                       OneWireMaster::StrongLevel);
    if (!result) {
      int sleepTime;
      switch (resolution) {
      case nineBitResolution:
        sleepTime = 94;
        break;

      case tenBitResolution:
        sleepTime = 188;
        break;

      case elevenBitResolution:
        sleepTime = 375;
        break;

      case twelveBitResolution:
      default:
        sleepTime = 750;
        break;
      }
      sleep->invoke(sleepTime);
      result = master->setLevel(OneWireMaster::NormalLevel);
    }
  }
  return result;
}

error_code DS18B20::recallEeprom() {  
  error_code result = selectRom(*master);
  if (!result) {
    result = master->writeByte(0xB8);
  }
  return result;
}

const error_category & DS18B20::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "DS18B20"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case CrcError:
        return "CRC Error";

      case DataError:
        return "Data Error";

      default:
        return defaultErrorMessage(condition);
      }
    }
  } instance;
  return instance;
}

error_code readTemperature(DS18B20 & ds18b20, int & temperature) {
  error_code result = ds18b20.convertTemperature();
  if (result) {
    return result;
  }
  DS18B20::Scratchpad::array scratchpad;
  result = ds18b20.readScratchpad(scratchpad);
  if (result) {
    return result;
  }

  const unsigned int tempData =
      (static_cast<unsigned int>(scratchpad[1]) << 8) | scratchpad[0];
  const unsigned int signMask = 0xF800;
  if ((tempData & signMask) == signMask) {
    temperature = -0x800;
  } else if ((tempData & signMask) == 0) {
    temperature = 0;
  } else {
    return make_error_code(DS18B20::DataError);
  }
  unsigned int precisionMask;
  switch (scratchpad[4]) {
  case DS18B20::nineBitResolution:
  default:
    precisionMask = 0x7;
    break;

  case DS18B20::tenBitResolution:
    precisionMask = 0x3;
    break;

  case DS18B20::elevenBitResolution:
    precisionMask = 0x1;
    break;

  case DS18B20::twelveBitResolution:
    precisionMask = 0x0;
    break;
  }
  temperature += static_cast<int>(tempData & ~(signMask | precisionMask));
  return error_code();
}

} // namespace MaximInterface
