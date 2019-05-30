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

#include <MaximInterface/Utilities/HexConversions.hpp>
#include "LoggingOneWireMaster.hpp"

using std::string;

namespace MaximInterface {

static const char strongLevelString[] = "<SP_ON>";

static string formatDataString(span<const uint_least8_t> data, bool read) {
  string dataBuilder;
  for (span<const uint_least8_t>::index_type i = 0; i < data.size(); ++i) {
    if (read) {
      dataBuilder.append(1, '[');
    }
    dataBuilder.append(byteArrayToHexString(data.subspan(i, 1)));
    if (read) {
      dataBuilder.append(1, ']');
    }
    dataBuilder.append(1, ' ');
  }
  return dataBuilder;
}

void LoggingOneWireMaster::tryWriteMessage(const std::string & message) {
  if (writeMessage) {
    writeMessage(message);
  }
}

error_code LoggingOneWireMaster::reset() {
  error_code result = OneWireMasterDecorator::reset();
  tryWriteMessage(!result ? "RP" : "RN");
  return result;
}

error_code LoggingOneWireMaster::writeByteSetLevel(uint_least8_t sendByte,
                                                   Level afterLevel) {
  tryWriteMessage(formatDataString(make_span(&sendByte, 1), false));
  if (afterLevel == StrongLevel) {
    tryWriteMessage(strongLevelString);
  }
  return OneWireMasterDecorator::writeByteSetLevel(sendByte, afterLevel);
}

error_code LoggingOneWireMaster::readByteSetLevel(uint_least8_t & recvByte,
                                                  Level afterLevel) {
  error_code result =
      OneWireMasterDecorator::readByteSetLevel(recvByte, afterLevel);
  if (!result) {
    tryWriteMessage(formatDataString(make_span(&recvByte, 1), true));
    if (afterLevel == StrongLevel) {
      tryWriteMessage(strongLevelString);
    }
  }
  return result;
}

error_code LoggingOneWireMaster::writeBlock(span<const uint_least8_t> sendBuf) {
  tryWriteMessage(formatDataString(sendBuf, false));
  return OneWireMasterDecorator::writeBlock(sendBuf);
}

error_code LoggingOneWireMaster::readBlock(span<uint_least8_t> recvBuf) {
  error_code result = OneWireMasterDecorator::readBlock(recvBuf);
  if (!result) {
    tryWriteMessage(formatDataString(recvBuf, true));
  }
  return result;
}

error_code LoggingOneWireMaster::setSpeed(Speed newSpeed) {
  error_code result = OneWireMasterDecorator::setSpeed(newSpeed);
  if (!result) {
    string newSpeedString;
    switch (newSpeed) {
    case StandardSpeed:
      newSpeedString = "<STD>";
      break;

    case OverdriveSpeed:
      newSpeedString = "<OVR>";
      break;
    }
    if (!newSpeedString.empty()) {
      tryWriteMessage(newSpeedString);
    }
  }
  return result;
}

error_code LoggingOneWireMaster::setLevel(Level newLevel) {
  error_code result = OneWireMasterDecorator::setLevel(newLevel);
  if (!result) {
    string newLevelString;
    switch (newLevel) {
    case NormalLevel:
      newLevelString = "<SP_OFF>";
      break;

    case StrongLevel:
      newLevelString = strongLevelString;
      break;
    }
    if (!newLevelString.empty()) {
      tryWriteMessage(newLevelString);
    }
  }
  return result;
}

} // namespace MaximInterface
