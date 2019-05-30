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

#ifndef MaximInterface_DS2431
#define MaximInterface_DS2431

#include <MaximInterface/Links/SelectRom.hpp>
#include <MaximInterface/Links/Sleep.hpp>
#include <MaximInterface/Utilities/array_span.hpp>
#include <MaximInterface/Utilities/Export.h>

namespace MaximInterface {

/// @brief DS2431 1024-bit 1-Wire EEPROM
/// @details The DS2431 is a 1024-bit, 1-Wire® EEPROM chip organized
/// as four memory pages of 256 bits each. Data is written to an 8-byte
/// scratchpad, verified, and then copied to the EEPROM memory. As a
/// special feature, the four memory pages can individually be write
/// protected or put in EPROM-emulation mode, where bits can only be
/// changed from a 1 to a 0 state. The DS2431 communicates over the
/// single-conductor 1-Wire bus. The communication follows the standard
/// 1-Wire protocol. Each device has its own unalterable and unique
/// 64-bit ROM registration number that is factory lasered into the chip.
/// The registration number is used to address the device in a multidrop,
/// 1-Wire net environment.
class DS2431 {
public:
  enum ErrorValue { CrcError = 1, OperationFailure };

  typedef array_span<uint_least8_t, 8> Scratchpad;
  typedef uint_least8_t Address;

  DS2431(Sleep & sleep, OneWireMaster & master, const SelectRom & selectRom)
      : selectRom(selectRom), master(&master), sleep(&sleep) {}

  void setSleep(Sleep & sleep) { this->sleep = &sleep; }
  
  void setMaster(OneWireMaster & master) { this->master = &master; }
  
  void setSelectRom(const SelectRom & selectRom) {
    this->selectRom = selectRom;
  }

  /// @brief Reads block of data from EEPROM memory.
  /// @param[in] beginAddress EEPROM memory address to start reading from.
  /// @param[out] data EEPROM data read from the device.
  MaximInterface_EXPORT error_code readMemory(Address beginAddress,
                                              span<uint_least8_t> data) const;

  /// @brief Writes 8 bytes to the scratchpad.
  /// @param[in] targetAddress
  /// EEPROM memory address that this data will be copied to.
  /// Must be on row boundary.
  /// @param[in] data Data to write to scratchpad.
  MaximInterface_EXPORT error_code writeScratchpad(Address targetAddress,
                                                   Scratchpad::const_span data);

  /// @brief Reads contents of scratchpad.
  /// @param[out] data Data read from scratchpad.
  /// @param[out] esByte E/S byte read before scratchpad data.
  MaximInterface_EXPORT error_code readScratchpad(Scratchpad::span data,
                                                  uint_least8_t & esByte);

  /// @brief Copies contents of scratchpad to EEPROM.
  /// @param[in] targetAddress EEPROM memory address that scratchpad
  /// will be copied to. Must be on row boundary.
  /// @param[in] esByte E/S byte from preceding Read Scratchpad command.
  MaximInterface_EXPORT error_code copyScratchpad(Address targetAddress,
                                                  uint_least8_t esByte);

  MaximInterface_EXPORT static const error_category & errorCategory();

private:
  SelectRom selectRom;
  OneWireMaster * master;
  const Sleep * sleep;
};

/// @brief
/// Writes data to EEPROM using Write Scratchpad, Read Scratchpad,
/// and Copy Scratchpad commands.
/// @param device Device to write.
/// @param[in] targetAddress EEPROM memory address to start writing at.
/// @param[in] data Data to write to EEPROM.
MaximInterface_EXPORT error_code writeMemory(DS2431 & device,
                                             DS2431::Address targetAddress,
                                             DS2431::Scratchpad::const_span data);

inline error_code make_error_code(DS2431::ErrorValue e) {
  return error_code(e, DS2431::errorCategory());
}

} // namespace MaximInterface

#endif
