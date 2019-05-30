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

#ifndef MaximInterface_DS2482_DS2484
#define MaximInterface_DS2482_DS2484

#include <MaximInterface/Links/OneWireMaster.hpp>
#include <MaximInterface/Links/I2CMaster.hpp>
#include <MaximInterface/Utilities/Export.h>

namespace MaximInterface {

/// Interface to the DS2484, DS2482-100, DS2482-101, DS2482-800 1-Wire masters.
class DS2482_DS2484 : public OneWireMaster {
public:
  enum ErrorValue { HardwareError = 1, ArgumentOutOfRangeError };

  /// Represents a device configuration.
  class Config {
  public:
    /// Default construct with power-on config.
    explicit Config(uint_least8_t readByte = optionAPU)
        : readByte_(readByte & 0xF) {}

    /// @name 1WS
    /// @brief 1-Wire Speed
    /// @{
    
    /// Get 1WS bit.
    bool get1WS() const { return (readByte_ & option1WS) == option1WS; }
    
    /// Set 1WS bit.
    Config & set1WS(bool new1WS) {
      if (new1WS) {
        readByte_ |= option1WS;
      } else {
        readByte_ &= ~option1WS;
      }
      return *this;
    }
    
    /// @}

    /// @name SPU
    /// @brief Strong Pullup
    /// @{
    
    /// Get SPU bit.
    bool getSPU() const { return (readByte_ & optionSPU) == optionSPU; }
    
    /// Set SPU bit.
    Config & setSPU(bool newSPU) {
      if (newSPU) {
        readByte_ |= optionSPU;
      } else {
        readByte_ &= ~optionSPU;
      }
      return *this;
    }
    
    /// @}

    /// @name PDN
    /// @brief 1-Wire Power Down
    /// @{
    
    /// Get PDN bit.
    bool getPDN() const { return (readByte_ & optionPDN) == optionPDN; }
    
    /// Set PDN bit.
    Config & setPDN(bool newPDN) {
      if (newPDN) {
        readByte_ |= optionPDN;
      } else {
        readByte_ &= ~optionPDN;
      }
      return *this;
    }
    
    /// @}

    /// @name APU
    /// @brief Active Pullup
    /// @{
    
    /// Get APU bit.
    bool getAPU() const { return (readByte_ & optionAPU) == optionAPU; }
    
    /// Set APU bit.
    Config & setAPU(bool newAPU) {
      if (newAPU) {
        readByte_ |= optionAPU;
      } else {
        readByte_ &= ~optionAPU;
      }
      return *this;
    }
    
    /// @}

    /// Byte representation that is read from the device.
    uint_least8_t readByte() const { return readByte_; }

  private:
    static const unsigned int option1WS = 0x8;
    static const unsigned int optionSPU = 0x4;
    static const unsigned int optionPDN = 0x2;
    static const unsigned int optionAPU = 0x1;

    uint_least8_t readByte_;
  };

  void setMaster(I2CMaster & master) { this->master = &master; }
  
  uint_least8_t address() const { return address_; }
  
  void setAddress(uint_least8_t address) { address_ = address; }

  /// Initialize hardware for use.
  MaximInterface_EXPORT error_code initialize(Config config = Config());

  /// @brief Write a new configuration to the device.
  /// @param[in] config New configuration to write.
  MaximInterface_EXPORT error_code writeConfig(Config config);

  MaximInterface_EXPORT virtual error_code triplet(TripletData & data);

  MaximInterface_EXPORT virtual error_code reset();
  
  MaximInterface_EXPORT virtual error_code touchBitSetLevel(bool & sendRecvBit,
                                                            Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  readByteSetLevel(uint_least8_t & recvByte, Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  writeByteSetLevel(uint_least8_t sendByte, Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code setSpeed(Speed newSpeed);
  
  MaximInterface_EXPORT virtual error_code setLevel(Level newLevel);

  MaximInterface_EXPORT static const error_category & errorCategory();

protected:
  DS2482_DS2484(I2CMaster & master, uint_least8_t address)
      : master(&master), address_(address) {}

  /// @note Allow marking const since not public.
  error_code sendCommand(uint_least8_t cmd) const;

  /// @note Allow marking const since not public.
  error_code sendCommand(uint_least8_t cmd, uint_least8_t param) const;

  /// @brief Reads a register from the device.
  /// @param reg Register to read from.
  /// @param[out] buf Buffer to hold read data.
  error_code readRegister(uint_least8_t reg, uint_least8_t & buf) const;

  /// @brief Reads the current register from the device.
  /// @param[out] buf Buffer to hold read data.
  error_code readRegister(uint_least8_t & buf) const;

private:
  /// @brief Performs a soft reset on the device.
  /// @note This is not a 1-Wire Reset.
  error_code resetDevice();

  /// @brief
  /// Polls the device status waiting for the 1-Wire Busy bit (1WB) to be cleared.
  /// @param[out] pStatus Optionally retrieve the status byte when 1WB cleared.
  /// @returns Success or TimeoutError if poll limit reached.
  error_code pollBusy(uint_least8_t * pStatus = NULL);

  /// @brief Ensure that the desired 1-Wire level is set in the configuration.
  /// @param level Desired 1-Wire level.
  error_code configureLevel(Level level);

  I2CMaster * master;
  uint_least8_t address_;
  Config curConfig;
};

inline error_code make_error_code(DS2482_DS2484::ErrorValue e) {
  return error_code(e, DS2482_DS2484::errorCategory());
}

class DS2482_100 : public DS2482_DS2484 {
public:
  DS2482_100(I2CMaster & i2c_bus, uint_least8_t adrs)
      : DS2482_DS2484(i2c_bus, adrs) {}
};

/// DS2482-800 I2C to 1-Wire Master
class DS2482_800 : public DS2482_DS2484 {
public:
  DS2482_800(I2CMaster & i2c_bus, uint_least8_t adrs)
      : DS2482_DS2484(i2c_bus, adrs) {}

  /// @brief Select the active 1-Wire channel.
  /// @param channel Channel number to select from 0 to 7.
  MaximInterface_EXPORT error_code selectChannel(int channel);
};

/// DS2484 I2C to 1-Wire Master
class DS2484 : public DS2482_DS2484 {
public:
  /// @brief 1-Wire port adjustment parameters.
  /// @note See datasheet page 13.
  enum PortParameter {
    tRSTL = 0,
    tRSTL_OD,
    tMSP,
    tMSP_OD,
    tW0L,
    tW0L_OD,
    tREC0,   // OD N/A
    RWPU = 8 // OD N/A
  };

  explicit DS2484(I2CMaster & i2c_bus, uint_least8_t adrs = 0x30)
      : DS2482_DS2484(i2c_bus, adrs) {}

  /// @brief Adjust 1-Wire port parameters.
  /// @param param Parameter to adjust.
  /// @param val New parameter value to set. Consult datasheet for value mappings.
  MaximInterface_EXPORT error_code adjustPort(PortParameter param, int val);
};

} // namespace MaximInterface

#endif
