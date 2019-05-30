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

#ifndef MaximInterface_I2CMaster
#define MaximInterface_I2CMaster

#include <stdint.h>
#include <MaximInterface/Utilities/Export.h>
#include <MaximInterface/Utilities/span.hpp>
#include <MaximInterface/Utilities/system_error.hpp>

namespace MaximInterface {

/// I2C master interface.
class I2CMaster {
public:
  enum ErrorValue {
    NackError = 1 ///< Transaction stopped due to a NACK from the slave device.
  };

  enum AckStatus { Nack, Ack };

  virtual ~I2CMaster() {}

  /// @brief Send start condition and address on the bus.
  /// @param address Address with R/W bit.
  virtual error_code start(uint_least8_t address) = 0;
  
  /// Send stop condition on the bus.
  virtual error_code stop() = 0;
  
  /// Write data byte to the bus.
  virtual error_code writeByte(uint_least8_t data) = 0;
  
  /// Write data block to the bus.
  MaximInterface_EXPORT virtual error_code
  writeBlock(span<const uint_least8_t> data);
  
  /// @brief
  /// Perform a complete write transaction on the bus with optional stop
  /// condition.
  /// @param address Address in 8-bit format.
  /// @param data Data to write to the bus.
  /// @param sendStop
  /// True to send a stop condition or false to set up a repeated start.
  error_code writePacket(uint_least8_t address, span<const uint_least8_t> data,
                         bool sendStop = true) {
    return writePacketImpl(address, data, sendStop);
  }
  
  /// @brief Read data byte from the bus.
  /// @param status Determines whether an ACK or NACK is sent after reading.
  /// @param[out] data Data read from the bus if successful.
  virtual error_code readByte(AckStatus status, uint_least8_t & data) = 0;
  
  /// @brief Read data block from the bus.
  /// @param status Determines whether an ACK or NACK is sent after reading.
  /// @param[out] data Data read from the bus if successful.
  MaximInterface_EXPORT virtual error_code
  readBlock(AckStatus status, span<uint_least8_t> data);
  
  /// @brief
  /// Perform a complete read transaction on the bus with optional stop
  /// condition.
  /// @param address Address in 8-bit format.
  /// @param[out] data Data read from the bus if successful.
  /// @param sendStop
  /// True to send a stop condition or false to set up a repeated start.
  error_code readPacket(uint_least8_t address, span<uint_least8_t> data,
                        bool sendStop = true) {
    return readPacketImpl(address, data, sendStop);
  }

  MaximInterface_EXPORT static const error_category & errorCategory();

protected:
  MaximInterface_EXPORT virtual error_code
  writePacketImpl(uint_least8_t address, span<const uint_least8_t> data,
                  bool sendStop);
 
  MaximInterface_EXPORT virtual error_code
  readPacketImpl(uint_least8_t address, span<uint_least8_t> data,
                 bool sendStop);
};

inline error_code make_error_code(I2CMaster::ErrorValue e) {
  return error_code(e, I2CMaster::errorCategory());
}

inline error_condition make_error_condition(I2CMaster::ErrorValue e) {
  return error_condition(e, I2CMaster::errorCategory());
}

} // namespace MaximInterface

#endif
