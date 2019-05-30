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

#include <stddef.h>
#include <algorithm>
#include <MaximInterface/Utilities/Algorithm.hpp>
#include <MaximInterface/Utilities/Error.hpp>
#include "DS28E38.hpp"

namespace MaximInterface {

using std::copy;

static const int readMemoryTimeMs = 30;
static const int writeMemoryTimeMs = 65;
static const int writeStateTimeMs = 15;
static const int generateEccKeyPairTimeMs = 200;
static const int generateEcdsaSignatureTimeMs = 130;
static const int trngOnDemandCheckTimeMs = 20;
static const int trngGenerationTimeMs = 10;

const int DS28E38::decrementCounterPage;
const int DS28E38::publicKeyXPage;
const int DS28E38::publicKeyYPage;
const int DS28E38::privateKeyPage;
const int DS28E38::memoryPages;

error_code DS28E38::writeMemory(int pageNum, Page::const_span page) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t command[2 + Page::size];
  command[0] = 0x96;
  command[1] = pageNum;
  copy(page.begin(), page.end(), command + 2);
  return runCommand(command, writeMemoryTimeMs);
}

error_code DS28E38::readMemory(int pageNum, Page::span page) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[1 + Page::size];
  buffer[0] = 0x44;
  buffer[1] = pageNum;
  span<uint_least8_t> response(buffer);
  const error_code result =
      runCommand(make_span(buffer, 2), readMemoryTimeMs, response);
  if (!result) {
    copy(response.begin(), response.end(), page.begin());
  }
  return result;
}

error_code DS28E38::readStatus(bool entropyHealthTest, Status & status) {
  int delay = readMemoryTimeMs;
  if (entropyHealthTest) {
    delay += trngOnDemandCheckTimeMs;
  }
  uint_least8_t buffer[Status::PageProtectionList::csize + ManId::size +
                       Status::RomVersion::csize + 2];
  buffer[0] = 0xAA;
  buffer[1] = entropyHealthTest ? 0x01 : 0x00;
  span<uint_least8_t> response(buffer);
  error_code result = runCommand(make_span(buffer, 2), delay, response);
  if (!result) {
    span<uint_least8_t>::const_iterator responseIt = response.begin();
    for (Status::PageProtectionList::iterator it =
             status.pageProtection.begin();
         it != status.pageProtection.end(); ++it) {
      *it = *responseIt;
      ++responseIt;
    }
    span<uint_least8_t>::const_iterator responseItEnd =
        responseIt + status.manId.size();
    copy(responseIt, responseItEnd, status.manId.begin());
    responseIt = responseItEnd;
    responseItEnd = responseIt + status.romVersion.size();
    copy(responseIt, responseItEnd, status.romVersion.begin());
    responseIt = responseItEnd;
    switch (*responseIt) {
    case Status::TestNotPerformed:
    case Status::EntropyHealthy:
    case Status::EntropyNotHealthy:
      status.entropyHealthTestStatus =
          static_cast<Status::EntropyHealthTestStatus>(*responseIt);
      break;

    default:
      result = make_error_code(InvalidResponseError);
      break;
    }
  }
  return result;
}

error_code DS28E38::setPageProtection(int pageNum,
                                      const PageProtection & protection) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  int delay = writeStateTimeMs;
  if (pageNum == decrementCounterPage) {
    delay += writeMemoryTimeMs;
  }
  const uint_least8_t command[] = {
      0xC3, static_cast<uint_least8_t>(pageNum),
      static_cast<uint_least8_t>(protection.to_ulong())};
  return runCommand(command, delay);
}

error_code
DS28E38::computeAndReadPageAuthentication(int pageNum, bool anonymous,
                                          Page::const_span challenge,
                                          Ecc256::Signature::span signature) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  const size_t commandSize = 2 + Page::size;
  const size_t responseSize = 1 + 2 * Ecc256::Scalar::size;
  uint_least8_t buffer[MaximInterface_MAX(commandSize, responseSize)];
  buffer[0] = 0xA5;
  buffer[1] = pageNum | (anonymous ? 0xE0 : 0x00);
  copy(challenge.begin(), challenge.end(), buffer + 2);
  span<uint_least8_t> response(buffer, responseSize);
  const error_code result = runCommand(make_span(buffer, commandSize),
                                       generateEcdsaSignatureTimeMs, response);
  if (!result) {
    span<uint_least8_t>::const_iterator begin = response.begin();
    span<uint_least8_t>::const_iterator end = begin + signature.s.size();
    copy(begin, end, signature.s.begin());
    begin = end;
    end = begin + signature.r.size();
    copy(begin, end, signature.r.begin());
  }
  return result;
}

error_code DS28E38::decrementCounter() {
  const uint_least8_t command = 0xC9;
  return runCommand(make_span(&command, 1), writeMemoryTimeMs);
}

error_code DS28E38::disableDevice() {
  const uint_least8_t command[] = {0x33, 0x9E, 0xA7, 0x49, 0xFB,
                                   0x10, 0x62, 0x0A, 0x26};
  return runCommand(command, writeStateTimeMs);
}

error_code DS28E38::generateEcc256KeyPair(bool privateKeyPuf,
                                          bool writeProtectEnable) {
  int delay = generateEccKeyPairTimeMs;
  if (writeProtectEnable) {
    delay += writeStateTimeMs;
  }
  uint_least8_t command[] = {0xCB, 0x00};
  if (privateKeyPuf) {
    command[1] |= 0x01;
  }
  if (writeProtectEnable) {
    command[1] |= 0x80;
  }
  return runCommand(command, delay);
}

error_code DS28E38::readRng(span<uint_least8_t> data) {
  const span<uint_least8_t>::index_type maxDataSize = 64;
  if ((data.size() < 1) || (data.size() > maxDataSize)) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[1 + maxDataSize];
  buffer[0] = 0xD2;
  buffer[1] = data.size() - 1;
  span<uint_least8_t> response(buffer, 1 + data.size());
  const error_code result =
      runCommand(make_span(buffer, 2), trngGenerationTimeMs, response);
  if (!result) {
    copy(response.begin(), response.end(), data.begin());
  }
  return result;
}

error_code DS28E38::runCommand(span<const uint_least8_t> command, int delayTime,
                               span<uint_least8_t> & response) {
  const span<const uint_least8_t>::index_type responseInputSize =
      response.size();
  error_code result = doRunCommand(command, delayTime, response);
  if (result) {
    return result;
  }
  if (response.empty()) {
    return make_error_code(InvalidResponseError);
  }
  // Parse command result byte.
  switch (response[0]) {
  case 0xAA:
    // Success response.
    if (response.size() != responseInputSize) {
      result = make_error_code(InvalidResponseError);
    }
    break;

  case 0x00:
    result = make_error_code(InvalidResponseError);
    break;

  default:
    result.assign(response[0], errorCategory());
    break;
  }
  response = response.subspan(1);
  return result;
}

error_code DS28E38::runCommand(span<const uint_least8_t> command,
                               int delayTime) {
  uint_least8_t buffer;
  span<uint_least8_t> response(&buffer, 1);
  return runCommand(command, delayTime, response);
}

const error_category & DS28E38::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "DS28E38"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case InvalidOperationError:
        return "Invalid Operation Error";

      case InvalidParameterError:
        return "Invalid Parameter Error";

      case InvalidSequenceError:
        return "Invalid Sequence Error";

      case InternalError:
        return "Internal Error";

      case DeviceDisabledError:
        return "Device Disabled Error";

      case InvalidResponseError:
        return "Invalid Response Error";
      }
      return defaultErrorMessage(condition);
    }
  } instance;
  return instance;
}

error_code readManId(DS28E38 & ds28e38, ManId::span manId) {
  DS28E38::Status status;
  const error_code result = ds28e38.readStatus(false, status);
  if (!result) {
    copy(make_span(status.manId), manId);
  }
  return result;
}

DS28E38::PageAuthenticationData &
DS28E38::PageAuthenticationData::setAnonymousRomId() {
  std::fill(romId().begin(), romId().end(), 0xFF);
  return *this;
}

} // namespace MaximInterface
