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
#include "DS2413.hpp"

namespace MaximInterface {

error_code DS2413::readStatus(Status & status) const {
  uint_least8_t val;
  const error_code result = pioAccessRead(val);
  if (!result) {
    status = val;
  }
  return result;
}

error_code DS2413::writeOutputState(bool pioAState, bool pioBState) {
  uint_least8_t val = 0xFC;
  if (pioAState) {
    val |= 0x1;
  }
  if (pioBState) {
    val |= 0x2;
  }
  return pioAccessWrite(val);
}

error_code DS2413::pioAccessRead(uint_least8_t & val) const {
  error_code result = selectRom(*master);
  if (!result) {
    result = master->writeByte(0xF5);
    if (!result) {
      result = master->readByte(val);
      if (!result && (val != ((val ^ 0xF0) >> 4))) {
        result = make_error_code(CommunicationError);
      }
    }
  }
  return result;
}

error_code DS2413::pioAccessWrite(uint_least8_t val) {
  error_code result = selectRom(*master);
  if (!result) {
    uint_least8_t block[] = {0x5A, val, static_cast<uint_least8_t>(val ^ 0xFF)};
    result = master->writeBlock(block);
    if (!result) {
      result = master->readByte(block[0]);
      if (!result && block[0] != 0xAA) {
        result = make_error_code(CommunicationError);
      }
    }
  }
  return result;
}

const error_category & DS2413::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "DS2413"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case CommunicationError:
        return "Communication Error";
      }
      return defaultErrorMessage(condition);
    }
  } instance;
  return instance;
}

error_code writePioAOutputState(DS2413 & ds2413, bool pioAState) {
  DS2413::Status status;
  error_code result = ds2413.readStatus(status);
  if (!result && pioAState != status[DS2413::PioAOutputState]) {
    result =
        ds2413.writeOutputState(pioAState, status[DS2413::PioBOutputState]);
  }
  return result;
}

error_code writePioBOutputState(DS2413 & ds2413, bool pioBState) {
  DS2413::Status status;
  error_code result = ds2413.readStatus(status);
  if (!result && pioBState != status[DS2413::PioBOutputState]) {
    result =
        ds2413.writeOutputState(status[DS2413::PioAOutputState], pioBState);
  }
  return result;
}

} // namespace MaximInterface
