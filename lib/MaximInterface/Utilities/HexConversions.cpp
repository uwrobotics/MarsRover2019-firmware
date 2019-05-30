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

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include "HexConversions.hpp"

using std::string;
using std::vector;

namespace MaximInterface {

static const int charsPerByte = 2;

string byteArrayToHexString(span<const uint_least8_t> byteArray) {
  string hexString;
  hexString.reserve(byteArray.size() * charsPerByte);
  for (span<const uint_least8_t>::index_type i = 0; i < byteArray.size(); ++i) {
    char hexBuf[charsPerByte + 1];
    std::sprintf(hexBuf, "%2.2X", byteArray[i] & 0xFF);
    hexString.append(hexBuf, charsPerByte);
  }
  return hexString;
}

optional<vector<uint_least8_t> >
hexStringToByteArray(const string & hexString) {
  optional<vector<uint_least8_t> > byteArray = vector<uint_least8_t>();
  byteArray->reserve(hexString.size() / charsPerByte);
  char hexBuf[charsPerByte + 1];
  char * const hexBufEnd = hexBuf + charsPerByte;
  *hexBufEnd = '\0';
  char * hexBufIt = hexBuf;
  for (string::const_iterator hexStringIt = hexString.begin(),
                              hexStringEnd = hexString.end();
       hexStringIt != hexStringEnd; ++hexStringIt) {
    if (!std::isspace(*hexStringIt)) {
      *hexBufIt = *hexStringIt;
      if (++hexBufIt == hexBufEnd) {
        char * hexBufParseEnd;
        byteArray->push_back(static_cast<uint_least8_t>(
            std::strtoul(hexBuf, &hexBufParseEnd, 16)));
        if (hexBufParseEnd != hexBufEnd) {
          break;
        }
        hexBufIt = hexBuf;
      }
    }
  }
  if (hexBufIt != hexBuf) {
    byteArray.reset();
  }
  return byteArray;
}

} // namespace MaximInterface
