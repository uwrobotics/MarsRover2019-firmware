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

#ifndef MaximInterface_DS2465
#define MaximInterface_DS2465

#include <MaximInterface/Links/I2CMaster.hpp>
#include <MaximInterface/Links/OneWireMaster.hpp>
#include <MaximInterface/Links/Sleep.hpp>
#include <MaximInterface/Utilities/array_span.hpp>
#include <MaximInterface/Utilities/Export.h>
#include <MaximInterface/Utilities/Sha256.hpp>

namespace MaximInterface {

/// Interface to the DS2465 1-Wire master and SHA-256 coprocessor.
class DS2465 : public OneWireMaster {
public:
  enum ErrorValue { HardwareError = 1, ArgumentOutOfRangeError };

  /// @brief 1-Wire port adjustment parameters.
  /// @note See datasheet page 13.
  enum PortParameter {
    tRSTL_STD,
    tRSTL_OD,
    tMSP_STD,
    tMSP_OD,
    tW0L_STD,
    tW0L_OD,
    tREC0,
    RWPU,
    tW1L_OD
  };

  /// Page region to use for swapping.
  enum PageRegion { FullPage = 0x03, FirstHalf = 0x01, SecondHalf = 0x02 };

  /// Holds the contents of a device memory segment.
  typedef array_span<uint_least8_t, 4> Segment;

  /// Holds the contents of a device memory page.
  typedef array_span<uint_least8_t, 32> Page;

  static const int memoryPages = 2;
  static const int segmentsPerPage = Page::size / Segment::size;

  /// Represents a DS2465 configuration.
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

    /// Byte representation that is read from the DS2465.
    uint_least8_t readByte() const { return readByte_; }

  private:
    static const unsigned int option1WS = 0x8;
    static const unsigned int optionSPU = 0x4;
    static const unsigned int optionPDN = 0x2;
    static const unsigned int optionAPU = 0x1;

    uint_least8_t readByte_;
  };

  // Const member functions should not change the settings of the DS2465 or
  // affect the state of the 1-Wire bus. Read pointer, scratchpad, MAC output
  // register, and command register on the DS2465 are considered mutable.

  DS2465(Sleep & sleep, I2CMaster & master, uint_least8_t address = 0x30)
      : sleep(&sleep), master(&master), address_(address & 0xFE) {}

  void setSleep(Sleep & sleep) { this->sleep = &sleep; }
  
  void setMaster(I2CMaster & master) { this->master = &master; }
  
  uint_least8_t address() const { return address_; }
  
  void setAddress(uint_least8_t address) { address_ = address & 0xFE; }

  /// Initialize hardware for use.
  MaximInterface_EXPORT error_code initialize(Config config = Config());

  /// @brief Write a new configuration to the DS2465.
  /// @param[in] config New configuration to write.
  MaximInterface_EXPORT error_code writeConfig(Config config);

  /// @brief Write a new port configuration parameter to the DS2465.
  /// @param[in] param Parameter to adjust.
  /// @param[in] val
  /// New parameter value to set. Consult datasheet for value mappings.
  MaximInterface_EXPORT error_code writePortParameter(PortParameter param,
                                                      int val);

  // 1-Wire Master Commands

  MaximInterface_EXPORT virtual error_code reset();
  
  MaximInterface_EXPORT virtual error_code touchBitSetLevel(bool & sendRecvBit,
                                                            Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  readByteSetLevel(uint_least8_t & recvByte, Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  writeByteSetLevel(uint_least8_t sendByte, Level afterLevel);
  
  MaximInterface_EXPORT virtual error_code
  readBlock(span<uint_least8_t> recvBuf);
  
  MaximInterface_EXPORT virtual error_code
  writeBlock(span<const uint_least8_t> sendBuf);
  
  MaximInterface_EXPORT virtual error_code setSpeed(Speed newSpeed);
  
  /// @copydoc OneWireMaster::setLevel
  /// @note
  /// The DS2465 only supports enabling strong pullup following a 1-Wire read or
  /// write operation.
  MaximInterface_EXPORT virtual error_code setLevel(Level newLevel);
  
  MaximInterface_EXPORT virtual error_code triplet(TripletData & data);

  // DS2465 Coprocessor Commands

  /// @brief Read data from an EEPROM memory page.
  /// @param pageNum Page number to read from.
  /// @param[out] data Data that was read.
  MaximInterface_EXPORT error_code readPage(int pageNum, Page::span data) const;

  /// @brief Write data to an EEPROM memory page.
  /// @param pageNum Page number to copy to.
  /// @param data Data to write.
  MaximInterface_EXPORT error_code writePage(int pageNum,
                                             Page::const_span data);

  /// @brief Write data to an EEPROM memory segment.
  /// @param pageNum Page number to copy to.
  /// @param segmentNum Segment number to copy to.
  /// @param data Data to write.
  MaximInterface_EXPORT error_code writeSegment(int pageNum, int segmentNum,
                                                Segment::const_span data);

  /// Write data to the secret EEPROM memory page.
  MaximInterface_EXPORT error_code
  writeMasterSecret(Sha256::Hash::const_span masterSecret);

  /// @brief Compute Next Master Secret.
  /// @param data Combined data fields for computation.
  MaximInterface_EXPORT error_code
  computeNextMasterSecret(Sha256::AuthenticationData::const_span data);

  /// @brief Compute Next Master Secret with page swapping.
  /// @param data Combined data fields for computation.
  /// @param pageNum Page number to swap in.
  /// @param region Region of the page to swap in.
  MaximInterface_EXPORT error_code
  computeNextMasterSecretWithSwap(Sha256::AuthenticationData::const_span data,
                                  int pageNum, PageRegion region);

  /// @brief Compute Write MAC.
  /// @param data Combined data fields for computation.
  /// @param[out] mac Computed Write MAC.
  MaximInterface_EXPORT error_code computeWriteMac(
      Sha256::WriteMacData::const_span data, Sha256::Hash::span mac) const;

  /// @brief Compute Write MAC.
  /// @param data Combined data fields for computation.
  MaximInterface_EXPORT error_code
  computeAndTransmitWriteMac(Sha256::WriteMacData::const_span data) const;

  /// @brief Compute Write MAC with page swapping.
  /// @param data Combined data fields for computation.
  /// @param pageNum Page number to swap in.
  /// @param segmentNum Segment number to swap in.
  /// @param[out] mac Computed Write MAC.
  MaximInterface_EXPORT error_code
  computeWriteMacWithSwap(Sha256::WriteMacData::const_span data, int pageNum,
                          int segmentNum, Sha256::Hash::span mac) const;

  /// @brief Compute Write MAC with page swapping.
  /// @param data Combined data fields for computation.
  /// @param pageNum Page number to swap in.
  /// @param segmentNum Segment number to swap in.
  MaximInterface_EXPORT error_code computeAndTransmitWriteMacWithSwap(
      Sha256::WriteMacData::const_span data, int pageNum, int segmentNum) const;

  /// @brief Compute Slave Secret (S-Secret).
  /// @param data Combined data fields for computation.
  MaximInterface_EXPORT error_code
  computeSlaveSecret(Sha256::AuthenticationData::const_span data);

  /// @brief Compute Slave Secret (S-Secret) with page swapping.
  /// @param data Combined data fields for computation.
  /// @param pageNum Page number to swap in.
  /// @param region Region of the page to swap in.
  MaximInterface_EXPORT error_code
  computeSlaveSecretWithSwap(Sha256::AuthenticationData::const_span data,
                             int pageNum, PageRegion region);

  /// @brief Compute Authentication MAC.
  /// @param data Combined data fields for computation.
  /// @param[out] mac Computed Auth MAC.
  MaximInterface_EXPORT error_code
  computeAuthMac(Sha256::AuthenticationData::const_span data,
                 Sha256::Hash::span mac) const;

  /// @brief Compute Authentication MAC.
  /// @param data Combined data fields for computation.
  MaximInterface_EXPORT error_code
  computeAndTransmitAuthMac(Sha256::AuthenticationData::const_span data) const;

  /// @brief Compute Authentication MAC with page swapping.
  /// @param data Combined data fields for computation.
  /// @param pageNum Page number to swap in.
  /// @param region Region of the page to swap in.
  /// @param[out] mac Computed Auth MAC.
  MaximInterface_EXPORT error_code computeAuthMacWithSwap(
      Sha256::AuthenticationData::const_span data, int pageNum,
      PageRegion region, Sha256::Hash::span mac) const;

  /// @brief Compute Authentication MAC with page swapping.
  /// @param data Combined data fields for computation.
  /// @param pageNum Page number to swap in.
  /// @param region Region of the page to swap in.
  MaximInterface_EXPORT error_code
  computeAndTransmitAuthMacWithSwap(Sha256::AuthenticationData::const_span data,
                                    int pageNum, PageRegion region) const;

  MaximInterface_EXPORT static const error_category & errorCategory();

private:
  const Sleep * sleep;
  I2CMaster * master;
  uint_least8_t address_;
  Config curConfig;

  /// @brief Performs a soft reset on the DS2465.
  /// @note This is not a 1-Wire Reset.
  error_code resetDevice();

  /// @brief
  /// Polls the DS2465 status waiting for the 1-Wire Busy bit (1WB) to be
  /// cleared.
  /// @param[out] pStatus Optionally retrive the status byte when 1WB cleared.
  /// @returns Success or TimeoutError if poll limit reached.
  error_code pollBusy(uint_least8_t * pStatus = NULL) const;

  /// @brief Ensure that the desired 1-Wire level is set in the configuration.
  /// @param level Desired 1-Wire level.
  error_code configureLevel(Level level);

  /// @note Const since only for internal use.
  error_code writeMemory(uint_least8_t addr,
                         span<const uint_least8_t> buf) const;

  /// @brief Read memory from the DS2465.
  /// @param addr Address to begin reading from.
  /// @param[out] buf Buffer to hold read data.
  error_code readMemory(uint_least8_t addr, span<uint_least8_t> buf) const;

  /// @brief Read memory from the DS2465 at the current pointer.
  /// @param[out] buf Buffer to hold read data.
  error_code readMemory(span<uint_least8_t> buf) const;

  /// Write the last computed MAC to the 1-Wire bus.
  error_code writeMacBlock() const;

  error_code computeWriteMac(Sha256::WriteMacData::const_span data) const;

  error_code computeWriteMacWithSwap(Sha256::WriteMacData::const_span data,
                                     int pageNum, int segmentNum) const;

  error_code computeAuthMac(Sha256::AuthenticationData::const_span data) const;

  error_code computeAuthMacWithSwap(Sha256::AuthenticationData::const_span data,
                                    int pageNum, PageRegion region) const;

  // Legacy implementations
  error_code copyScratchpad(bool destSecret, int pageNum, bool notFull,
                            int segmentNum);

  error_code computeNextMasterSecret(bool swap, int pageNum, PageRegion region);

  error_code computeWriteMac(bool regwrite, bool swap, int pageNum,
                             int segmentNum) const;

  error_code computeSlaveSecret(bool swap, int pageNum, PageRegion region);

  error_code computeAuthMac(bool swap, int pageNum, PageRegion region) const;
};

inline error_code make_error_code(DS2465::ErrorValue e) {
  return error_code(e, DS2465::errorCategory());
}

} // namespace MaximInterface

#endif
