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

#ifndef MaximInterface_LoggingOneWireMaster
#define MaximInterface_LoggingOneWireMaster

#include <string>
#include <MaximInterface/Links/OneWireMasterDecorator.hpp>
#include <MaximInterface/Utilities/Export.h>
#include <MaximInterface/Utilities/WriteMessage.hpp>

namespace MaximInterface {

class LoggingOneWireMaster : public OneWireMasterDecorator {
public:
  explicit LoggingOneWireMaster(
      OneWireMaster & master,
      const WriteMessage & writeMessage = WriteMessage())
      : OneWireMasterDecorator(master), writeMessage(writeMessage) {}

  void setWriteMessage(const WriteMessage & writeMessage) {
    this->writeMessage = writeMessage;
  }

  MaximInterface_EXPORT virtual error_code reset();
  
  MaximInterface_EXPORT virtual error_code
  writeByteSetLevel(uint_least8_t sendByte, Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  readByteSetLevel(uint_least8_t & recvByte, Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  writeBlock(span<const uint_least8_t> sendBuf);
  
  MaximInterface_EXPORT virtual error_code
  readBlock(span<uint_least8_t> recvBuf);
  
  MaximInterface_EXPORT virtual error_code setSpeed(Speed newSpeed);
  
  MaximInterface_EXPORT virtual error_code setLevel(Level newLevel);

private:
  void tryWriteMessage(const std::string & message);

  WriteMessage writeMessage;
};

} // namespace MaximInterface

#endif
