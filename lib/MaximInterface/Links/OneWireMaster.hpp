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

#ifndef MaximInterface_OneWireMaster
#define MaximInterface_OneWireMaster

#include <stdint.h>
#include <MaximInterface/Utilities/Export.h>
#include <MaximInterface/Utilities/span.hpp>
#include <MaximInterface/Utilities/system_error.hpp>

namespace MaximInterface {

/// 1-Wire master interface.
class OneWireMaster {
public:
  /// Speed of the 1-Wire bus.
  enum Speed { StandardSpeed = 0x00, OverdriveSpeed = 0x01 };

  /// Level of the 1-Wire bus.
  enum Level { NormalLevel = 0x00, StrongLevel = 0x02 };

  /// Result of all 1-Wire commands.
  enum ErrorValue {
    NoSlaveError = 1, ///< Slave not detected, typically due to no presence pulse.
    ShortDetectedError,
    InvalidSpeedError,
    InvalidLevelError
  };

  struct TripletData {
    bool writeBit;
    bool readBit;
    bool readBitComplement;
  };

  virtual ~OneWireMaster() {}

  /// @brief
  /// Reset all of the devices on the 1-Wire bus and check for a presence pulse.
  /// @returns
  /// NoSlaveError if reset was performed but no presence pulse was detected.
  virtual error_code reset() = 0;

  /// @brief
  /// Send and receive one bit of communication and set a new level on the
  /// 1-Wire bus.
  /// @param[in,out] sendRecvBit
  /// Input containing the bit to send and output containing the received bit.
  /// @param afterLevel Level to set the 1-Wire bus to after communication.
  virtual error_code touchBitSetLevel(bool & sendRecvBit, Level afterLevel) = 0;

  /// @brief
  /// Send one byte of communication and set a new level on the 1-Wire bus.
  /// @param sendByte Byte to send on the 1-Wire bus.
  /// @param afterLevel Level to set the 1-Wire bus to after communication.
  MaximInterface_EXPORT virtual error_code
  writeByteSetLevel(uint_least8_t sendByte, Level afterLevel);

  /// @brief
  /// Receive one byte of communication and set a new level on the 1-Wire bus.
  /// @param recvByte Buffer to receive the data from the 1-Wire bus.
  /// @param afterLevel Level to set the 1-Wire bus to after communication.
  MaximInterface_EXPORT virtual error_code
  readByteSetLevel(uint_least8_t & recvByte, Level afterLevel);

  /// @brief Send a block of communication on the 1-Wire bus.
  /// @param[in] sendBuf Buffer to send on the 1-Wire bus.
  MaximInterface_EXPORT virtual error_code
  writeBlock(span<const uint_least8_t> sendBuf);

  /// @brief Receive a block of communication on the 1-Wire bus.
  /// @param[out] recvBuf Buffer to receive the data from the 1-Wire bus.
  MaximInterface_EXPORT virtual error_code
  readBlock(span<uint_least8_t> recvBuf);

  /// Set the 1-Wire bus communication speed.
  virtual error_code setSpeed(Speed newSpeed) = 0;

  /// Set the 1-Wire bus level.
  virtual error_code setLevel(Level newLevel) = 0;

  /// @brief 1-Wire Triplet operation.
  /// @details Perform one bit of a 1-Wire search. This command
  /// does two read bits and one write bit. The write bit is either
  /// the default direction (all devices have same bit) or in case
  /// of a discrepancy, the data.writeBit parameter is used.
  /// @param[in,out] data
  /// Input with desired writeBit in case both read bits are zero.
  /// Output with all data fields set.
  MaximInterface_EXPORT virtual error_code triplet(TripletData & data);

  /// @brief
  /// Send one bit of communication and set a new level on the 1-Wire bus.
  /// @param sendBit Bit to send on the 1-Wire bus.
  /// @param afterLevel Level to set the 1-Wire bus to after communication.
  error_code writeBitSetLevel(bool sendBit, Level afterLevel) {
    return touchBitSetLevel(sendBit, afterLevel);
  }

  /// @brief
  /// Receive one bit of communication and set a new level on the 1-Wire bus.
  /// @param[out] recvBit Received data from the 1-Wire bus.
  /// @param afterLevel Level to set the 1-Wire bus to after communication.
  error_code readBitSetLevel(bool & recvBit, Level afterLevel) {
    recvBit = 1;
    return touchBitSetLevel(recvBit, afterLevel);
  }

  // Alternate forms of the read and write functions.
  
  error_code touchBit(bool & sendRecvBit) {
    return touchBitSetLevel(sendRecvBit, NormalLevel);
  }
  
  error_code writeBit(bool sendBit) {
    return writeBitSetLevel(sendBit, NormalLevel);
  }
  
  error_code readBit(bool & recvBit) {
    return readBitSetLevel(recvBit, NormalLevel);
  }
  
  error_code writeBitPower(bool sendBit) {
    return writeBitSetLevel(sendBit, StrongLevel);
  }
  
  error_code readBitPower(bool & recvBit) {
    return readBitSetLevel(recvBit, StrongLevel);
  }
  
  error_code writeByte(uint_least8_t sendByte) {
    return writeByteSetLevel(sendByte, NormalLevel);
  }
  
  error_code readByte(uint_least8_t & recvByte) {
    return readByteSetLevel(recvByte, NormalLevel);
  }
  
  error_code writeBytePower(uint_least8_t sendByte) {
    return writeByteSetLevel(sendByte, StrongLevel);
  }
  
  error_code readBytePower(uint_least8_t & recvByte) {
    return readByteSetLevel(recvByte, StrongLevel);
  }

  MaximInterface_EXPORT static const error_category & errorCategory();
};

inline error_code make_error_code(OneWireMaster::ErrorValue e) {
  return error_code(e, OneWireMaster::errorCategory());
}

inline error_condition make_error_condition(OneWireMaster::ErrorValue e) {
  return error_condition(e, OneWireMaster::errorCategory());
}

} // namespace MaximInterface

#endif
