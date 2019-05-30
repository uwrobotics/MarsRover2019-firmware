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

#ifndef MaximInterface_RomId
#define MaximInterface_RomId

#include "array_span.hpp"
#include "crc.hpp"

namespace MaximInterface {

/// Standard container for a 1-Wire ROM ID.
typedef array_span<uint_least8_t, 8> RomId;

/// @name Family Code
/// @{

/// Get the Family Code byte.
inline RomId::element familyCode(RomId::const_span romId) {
  return romId[0];
}

/// Set the Family Code byte.
inline void setFamilyCode(RomId::span romId, RomId::element familyCode) {
  romId[0] = familyCode;
}

/// @}

/// @name CRC8
/// @{

/// Get the CRC8 byte.
inline RomId::element crc8(RomId::const_span romId) { 
  return *romId.last<1>().data();
}

/// Set the CRC8 byte.
inline void setCrc8(RomId::span romId, RomId::element crc8) {
  *romId.last<1>().data() = crc8;
}

/// @}

/// @brief Check if the ROM ID is valid (Family Code and CRC8 are both valid).
/// @returns True if the ROM ID is valid.
inline bool valid(RomId::const_span romId) {
  return calculateCrc8(romId.first(romId.size() - 1)) == crc8(romId);
}

} // namespace MaximInterface

#endif
