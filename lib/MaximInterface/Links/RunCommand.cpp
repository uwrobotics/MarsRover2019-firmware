/*******************************************************************************
* Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
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

#include <MaximInterface/Utilities/crc.hpp>
#include <MaximInterface/Utilities/Error.hpp>
#include "I2CMaster.hpp"
#include "OneWireMaster.hpp"
#include "Sleep.hpp"
#include "RunCommand.hpp"

namespace MaximInterface {

const error_category & RunCommandWithOneWireMaster::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "RunCommandWithOneWireMaster"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case CrcError:
        return "CRC Error";

      case InvalidResponseError:
        return "Invalid Response Error";
      }
      return defaultErrorMessage(condition);
    }
  } instance;
  return instance;
}

error_code RunCommandWithOneWireMaster::
operator()(span<const uint_least8_t> command, int delayTime,
           span<uint_least8_t> & response) const {
  // Write command.
  error_code result = selectRom(*master);
  if (result) {
    return result;
  }
  uint_least8_t xpcBuffer[2] = {0x66,
                                static_cast<uint_least8_t>(command.size())};
  result = master->writeBlock(xpcBuffer);
  if (result) {
    return result;
  }
  result = master->writeBlock(command);
  if (result) {
    return result;
  }
  uint_fast16_t expectedCrc =
      calculateCrc16(command, calculateCrc16(xpcBuffer)) ^ 0xFFFFU;
  result = master->readBlock(xpcBuffer);
  if (result) {
    return result;
  }
  if (expectedCrc !=
      ((static_cast<uint_fast16_t>(xpcBuffer[1]) << 8) | xpcBuffer[0])) {
    return make_error_code(CrcError);
  }
  result = master->writeBytePower(0xAA);
  if (result) {
    return result;
  }

  // Wait for device to process.
  sleep->invoke(delayTime);

  // Read response.
  result = master->setLevel(OneWireMaster::NormalLevel);
  if (result) {
    return result;
  }
  result = master->readBlock(xpcBuffer);
  if (result) {
    return result;
  }
  if (xpcBuffer[1] > response.size()) {
    return make_error_code(InvalidResponseError);
  }
  response = response.first(xpcBuffer[1]);
  result = master->readBlock(response);
  if (result) {
    return result;
  }
  expectedCrc =
      calculateCrc16(response, calculateCrc16(make_span(xpcBuffer + 1, 1))) ^
      0xFFFFU;
  result = master->readBlock(xpcBuffer);
  if (result) {
    return result;
  }
  if (expectedCrc !=
      ((static_cast<uint_fast16_t>(xpcBuffer[1]) << 8) | xpcBuffer[0])) {
    return make_error_code(CrcError);
  }
  return result;
}

const error_category & RunCommandWithI2CMaster::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "RunCommandWithI2CMaster"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case InvalidResponseError:
        return "Invalid Response Error";
      }
      return defaultErrorMessage(condition);
    }
  } instance;
  return instance;
}

error_code RunCommandWithI2CMaster::
operator()(span<const uint_least8_t> command, int delayTime,
           span<uint_least8_t> & response) const {
  // Write command.
  error_code result = master->start(address_);
  if (result == make_error_condition(I2CMaster::NackError) && address_ != 0) {
    result = master->start(0);
  }
  if (result) {
    master->stop();
    return result;
  }
  if (!command.empty()) {
    result = master->writeByte(command[0]);
    if (result) {
      master->stop();
      return result;
    }
    command = command.subspan(1);
    if (!command.empty()) {
      result = master->writeByte(command.size());
      if (result) {
        master->stop();
        return result;
      }
      result = master->writeBlock(command);
      if (result) {
        master->stop();
        return result;
      }
    }
  }
  result = master->stop();
  if (result) {
    return result;
  }

  // Wait for device to process.
  sleep->invoke(delayTime);

  // Read response.
  result = master->start(address_ | 1);
  if (result) {
    master->stop();
    return result;
  }
  uint_least8_t length;
  result = master->readByte(I2CMaster::Ack, length);
  if (result) {
    master->stop();
    return result;
  }
  if (length > response.size()) {
    master->stop();
    return make_error_code(InvalidResponseError);
  }
  response = response.first(length);
  result = master->readBlock(I2CMaster::Nack, response);
  if (result) {
    master->stop();
    return result;
  }
  result = master->stop();
  return result;
}

} // namespace MaximInterface
