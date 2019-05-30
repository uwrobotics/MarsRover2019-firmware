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

#ifndef MaximInterface_DS28E17
#define MaximInterface_DS28E17

#include <stdint.h>
#include <MaximInterface/Links/SelectRom.hpp>
#include <MaximInterface/Utilities/Export.h>
#include <MaximInterface/Utilities/span.hpp>

namespace MaximInterface {

/// @brief DS28E17 1-Wire®-to-I2C Master Bridge
/// @details The DS28E17 is a 1-Wire slave to I2C master bridge
/// device that interfaces directly to I2C slaves at standard
/// (100kHz max) or fast (400kHz max). Data transfers serially by
/// means of the 1-Wire® protocol, which requires only a single data
/// lead and a ground return. Every DS28E17 is guaranteed to have a
/// unique 64-bit ROM registration number that serves as a node
/// address in the 1-Wire network. Multiple DS28E17 devices can
/// coexist with other devices in the 1-Wire network and be accessed
/// individually without affecting other devices. The DS28E17 allows
/// using complex I2C devices such as display controllers, ADCs, DACs,
/// I2C sensors, etc. in a 1-Wire environment. Each self-timed DS28E17
/// provides 1-Wire access for a single I2C interface.
class DS28E17 {
public:
  enum ErrorValue {
    TimeoutError = 1,
    OutOfRangeError,
    InvalidCrc16Error,
    AddressNackError,
    InvalidStartError,
    WriteNackError
  };

  enum I2CSpeed { Speed100kHz, Speed400kHz, Speed900kHz };

  DS28E17(OneWireMaster & master, const SelectRom & selectRom)
      : selectRom(selectRom), master(&master) {}

  void setMaster(OneWireMaster & master) { this->master = &master; }
  
  void setSelectRom(const SelectRom & selectRom) {
    this->selectRom = selectRom;
  }

  /// @brief Write Data With Stop command.
  /// @details Output on I2C: S, Address + Write, Write Data [1-255], P
  /// @param[in] I2C_addr
  /// I2C slave address. The least significant bit of the I2C
  /// address is automatically cleared by the command.
  /// @param[in] data I2C data to write with length 1-255.
  /// @param[out] wr_status
  /// Indicates which write byte NACK’d. A value of 00h indicates all bytes
  /// were acknowledged by the slave. A non-zero value indicates the byte number
  /// that NACK’d. May be set to NULL.
  MaximInterface_EXPORT error_code
  writeDataWithStop(uint_least8_t I2C_addr, span<const uint_least8_t> data,
                    uint_least8_t * wr_status = NULL);

  /// @brief Write Data No Stop command.
  /// @details Output on I2C: S, Address + Write, Write Data [1-255]
  /// @param[in] I2C_addr
  /// I2C slave address. The least significant bit of the I2C address
  /// is automatically cleared by the command.
  /// @param[in] data I2C data to write with length 1-255.
  /// @param[out] wr_status
  /// Indicates which write byte NACK’d. A value of 00h indicates all bytes
  /// were acknowledged by the slave. A non-zero value indicates the byte number
  /// that NACK’d. May be set to NULL.
  MaximInterface_EXPORT error_code
  writeDataNoStop(uint_least8_t I2C_addr, span<const uint_least8_t> data,
                  uint_least8_t * wr_status = NULL);

  /// @brief Write Data Only command.
  /// @details Output on I2C: Write Data [1-255]
  /// @param[in] data I2C data to write with length 1-255.
  /// @param[out] wr_status
  /// Indicates which write byte NACK’d. A value of 00h indicates all bytes
  /// were acknowledged by the slave. A non-zero value indicates the byte number
  /// that NACK’d. May be set to NULL.
  MaximInterface_EXPORT error_code
  writeDataOnly(span<const uint_least8_t> data,
                uint_least8_t * wr_status = NULL);

  /// @brief Write Data Only With Stop command.
  /// @details Output on I2C: Write Data [1-255], P
  /// @param[in] data I2C data to write with length 1-255.
  /// @param[out] wr_status
  /// Indicates which write byte NACK’d. A value of 00h indicates all bytes
  /// were acknowledged by the slave. A non-zero value indicates the byte number
  /// that NACK’d. May be set to NULL.
  MaximInterface_EXPORT error_code
  writeDataOnlyWithStop(span<const uint_least8_t> data,
                        uint_least8_t * wr_status = NULL);

  /// @brief Write, Read Data With Stop command.
  /// @details Output on I2C:
  /// S, Slave Address + Write, Write Data [1-255],
  /// Sr, Address + Read, Read Data [1-255], P (NACK last read byte)
  /// @param[in] I2C_addr
  /// I2C slave address. The least significant bit of the I2C address
  /// is automatically cleared and set by the command.
  /// @param[in] write_data I2C data to write with length 1-255.
  /// @param[out] read_data I2C data that was read with length 1-255.
  /// @param[out] wr_status
  /// Indicates which write byte NACK’d. A value of 00h indicates all bytes
  /// were acknowledged by the slave. A non-zero value indicates the byte number
  /// that NACK’d. May be set to NULL.
  MaximInterface_EXPORT error_code writeReadDataWithStop(
      uint_least8_t I2C_addr, span<const uint_least8_t> write_data,
      span<uint_least8_t> read_data, uint_least8_t * wr_status = NULL);

  /// @brief Read Data With Stop command.
  /// @details Output on I2C:
  /// S, Slave Address + Read, Read Data [1-255], P (NACK last read byte)
  /// @param[in]  I2C_addr
  /// I2C slave address. The least significant bit of the I2C address
  /// is automatically set by the command.
  /// @param[out] data I2C data that was read with length 1-255.
  MaximInterface_EXPORT error_code readDataWithStop(uint_least8_t I2C_addr,
                                                    span<uint_least8_t> data);

  /// Write to Configuration Register of DS28E17.
  MaximInterface_EXPORT error_code writeConfigReg(I2CSpeed speed);

  /// @brief Read the Configuration Register of DS28E17.
  /// @param[out] speed Speed read from configuration register.
  MaximInterface_EXPORT error_code readConfigReg(I2CSpeed & speed);

  /// @brief Put the device into a low current mode.
  /// @details All 1-Wire communication is ignored until woken up. Immediately
  /// after the command, the device monitors the WAKEUP input pin and exits
  /// sleep mode on a rising edge.
  MaximInterface_EXPORT error_code enableSleepMode();

  /// @brief Read the Device Revision of DS28E17.
  /// @details The upper nibble is the major revision,
  /// and the lower nibble is the minor revision.
  /// @param[out] rev Device Revision.
  MaximInterface_EXPORT error_code readDeviceRevision(uint_least8_t & rev);

  MaximInterface_EXPORT static const error_category & errorCategory();

private:
  enum Command {
    WriteDataWithStopCmd = 0x4B,
    WriteDataNoStopCmd = 0x5A,
    WriteDataOnlyCmd = 0x69,
    WriteDataOnlyWithStopCmd = 0x78,
    ReadDataWithStopCmd = 0x87,
    WriteReadDataWithStopCmd = 0x2D,
    WriteConfigurationCmd = 0xD2,
    ReadConfigurationCmd = 0xE1,
    EnableSleepModeCmd = 0x1E,
    ReadDeviceRevisionCmd = 0xC3
  };

  error_code sendPacket(Command command, const uint_least8_t * I2C_addr,
                        span<const uint_least8_t> write_data,
                        span<uint_least8_t> read_data,
                        uint_least8_t * wr_status);

  SelectRom selectRom;
  OneWireMaster * master;
};

inline error_code make_error_code(DS28E17::ErrorValue e) {
  return error_code(e, DS28E17::errorCategory());
}

} // namespace MaximInterface

#endif
