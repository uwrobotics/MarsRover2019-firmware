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

#ifndef MaximInterface_DS28E15_22_25
#define MaximInterface_DS28E15_22_25

#include <stdint.h>
#include <MaximInterface/Links/OneWireMaster.hpp>
#include <MaximInterface/Links/SelectRom.hpp>
#include <MaximInterface/Links/Sleep.hpp>
#include <MaximInterface/Utilities/array_span.hpp>
#include <MaximInterface/Utilities/Export.h>
#include <MaximInterface/Utilities/ManId.hpp>
#include <MaximInterface/Utilities/Sha256.hpp>

namespace MaximInterface {

/// @brief
/// Interface to the DS28E15/22/25 series of authenticators
/// including low power variants.
class DS28E15_22_25 {
public:
  enum ErrorValue { CrcError = 1, OperationFailure };

  /// Holds the contents of a device memory segment.
  typedef array_span<uint_least8_t, 4> Segment;

  /// Holds the contents of a device memory page.
  typedef array_span<uint_least8_t, 32> Page;

  /// Number of segments per page.
  static const int segmentsPerPage = Page::size / Segment::size;

  /// Holds the contents of the device scratchpad.
  typedef array_span<uint_least8_t, 32> Scratchpad;

  /// Container for the device personality.
  struct Personality {
    uint_least8_t PB1;
    uint_least8_t PB2;
    ManId::array manId;

    bool secretLocked() const { return PB2 & 0x01; }
  };

  // Represents the status of a memory protection block.
  class BlockProtection;

  // Format data to hash for an Authenticated Write to a memory segment.
  class SegmentWriteMacData;

  // Format data to hash for an Authenticated Write to a memory protection block.
  class ProtectionWriteMacData;

  // Format data to hash for device authentication or computing the next secret
  // from the existing secret.
  class AuthenticationData;

  void setSleep(Sleep & sleep) { this->sleep = &sleep; }
  
  void setMaster(OneWireMaster & master) { this->master = &master; }
  
  void setSelectRom(const SelectRom & selectRom) {
    this->selectRom = selectRom;
  }

  // Const member functions should not affect the state of the memory,
  // block protection, or secret on the device.

  /// @brief Read memory segment using the Read Memory command on the device.
  /// @param pageNum Page number for read operation.
  /// @param segmentNum Segment number within page for read operation.
  /// @param[out] data Buffer to read data from the segment into.
  MaximInterface_EXPORT error_code readSegment(int pageNum, int segmentNum,
                                               Segment::span data) const;

  /// @brief Continue an in-progress readSegment operation.
  /// @note A CRC16 will encountered after reading the last segment of a page.
  /// @param[out] data Buffer to read data from the segment into.
  MaximInterface_EXPORT error_code
  continueReadSegment(Segment::span data) const;

  /// @brief Write memory segment using the Write Memory command.
  /// @note 1-Wire ROM selection should have already occurred.
  /// @param pageNum Page number for write operation.
  /// @param segmentNum Segment number within page for write operation.
  /// @param[in] data Data to write to the memory segment.
  MaximInterface_EXPORT error_code writeSegment(int pageNum, int segmentNum,
                                                Segment::const_span data);

  /// @brief Continue an in-progress Write Memory command.
  /// @param[in] data Data to write to the memory segment.
  MaximInterface_EXPORT error_code
  continueWriteSegment(Segment::const_span data);

  /// @brief Read memory page using the Read Memory command on the device.
  /// @param pageNum Page number for write operation.
  /// @param[out] rdbuf Buffer to read data from the page into.
  MaximInterface_EXPORT error_code readPage(int pageNum,
                                            Page::span rdbuf) const;

  /// @brief Continue an in-progress readPageOperation.
  /// @param[out] rdbuf Buffer to read data from the page into.
  MaximInterface_EXPORT error_code continueReadPage(Page::span rdbuf) const;

  /// @brief
  /// Perform a Compute Page MAC command on the device.
  /// Read back the MAC and verify the CRC16.
  /// @param pageNum Page number to use for the computation.
  /// @param anon True to compute in anonymous mode where ROM ID is not used.
  /// @param[out] mac The device computed MAC.
  MaximInterface_EXPORT error_code
  computeReadPageMac(int pageNum, bool anon, Sha256::Hash::span mac) const;

  /// @brief
  /// Update the status of a memory protection block using the
  /// Write Page Protection command.
  /// @param protection
  /// Desired protection status for the block.
  /// It is not possible to disable existing protections.
  MaximInterface_EXPORT error_code
  writeBlockProtection(BlockProtection protection);

  /// @brief
  /// Update the status of a memory protection block using the
  /// Authenticated Write Page Protection command.
  /// @param newProtection New protection status to write.
  /// @param[in] mac Write MAC computed for this operation.
  MaximInterface_EXPORT error_code writeAuthBlockProtection(
      BlockProtection newProtection, Sha256::Hash::const_span mac);

  /// @brief Perform Load and Lock Secret command on the device.
  /// @note The secret should already be stored in the scratchpad on the device.
  /// @param lock
  /// Prevent further changes to the secret on the device after loading.
  MaximInterface_EXPORT error_code loadSecret(bool lock);

  /// @brief Perform a Compute and Lock Secret command on the device.
  /// @param pageNum Page number to use as the binding data.
  /// @param lock
  /// Prevent further changes to the secret on the device after computing.
  MaximInterface_EXPORT error_code computeSecret(int pageNum, bool lock);

  /// @brief Read the personality bytes using the Read Status command.
  /// @param[out] personality Receives personality read from device.
  MaximInterface_EXPORT error_code
  readPersonality(Personality & personality) const;

  MaximInterface_EXPORT static const error_category & errorCategory();

protected:
  enum Variant { DS28E15, DS28E22, DS28E25 };

  DS28E15_22_25(Sleep & sleep, OneWireMaster & master,
                const SelectRom & selectRom)
      : selectRom(selectRom), master(&master), sleep(&sleep) {}
      
  ~DS28E15_22_25() {}

  error_code doWriteScratchpad(Scratchpad::const_span data, Variant variant);

  error_code doReadScratchpad(Scratchpad::span data, Variant variant) const;

  error_code doReadBlockProtection(int blockNum, BlockProtection & protection,
                                   Variant variant) const;

  error_code doWriteAuthSegment(int pageNum, int segmentNum,
                                Segment::const_span newData,
                                Sha256::Hash::const_span mac, Variant variant);

  error_code doContinueWriteAuthSegment(Segment::const_span newData,
                                        Sha256::Hash::const_span mac,
                                        Variant variant);

  error_code doReadAllBlockProtection(span<BlockProtection> protection,
                                      Variant variant) const;

  error_code doLoadSecret(bool lock, bool lowPower);

  error_code doComputeSecret(int pageNum, bool lock, bool lowPower);

private:
  enum Command {
    WriteMemory = 0x55,
    ReadMemory = 0xF0,
    LoadAndLockSecret = 0x33,
    ComputeAndLockSecret = 0x3C,
    ReadWriteScratchpad = 0x0F,
    ComputePageMac = 0xA5,
    ReadStatus = 0xAA,
    WriteBlockProtection = 0xC3,
    AuthWriteMemory = 0x5A,
    AuthWriteBlockProtection = 0xCC,
  };

  error_code doWriteAuthSegment(Segment::const_span newData,
                                Sha256::Hash::const_span mac, Variant variant,
                                bool continuing);

  error_code writeCommandWithCrc(
      Command command, uint_least8_t parameter,
      OneWireMaster::Level level = OneWireMaster::NormalLevel) const;

  SelectRom selectRom;
  OneWireMaster * master;
  Sleep * sleep;
};

inline error_code make_error_code(DS28E15_22_25::ErrorValue e) {
  return error_code(e, DS28E15_22_25::errorCategory());
}

/// Interface to the DS28EL15 (low power) authenticator.
class DS28EL15 : public DS28E15_22_25 {
public:
  // DS28E15_22_25 traits
  static const int memoryPages = 2;
  static const int protectionBlocks = 4;

  DS28EL15(Sleep & sleep, OneWireMaster & master, const SelectRom & selectRom)
      : DS28E15_22_25(sleep, master, selectRom) {}

  /// @brief Perform Write Scratchpad operation on the device.
  /// @param[in] data Data to write to the scratchpad.
  MaximInterface_EXPORT error_code writeScratchpad(Scratchpad::const_span data);

  /// @brief Perform a Read Scratchpad operation on the device.
  /// @param[out] data Buffer to read data from the scratchpad into.
  MaximInterface_EXPORT error_code readScratchpad(Scratchpad::span data) const;

  /// @brief
  /// Read the status of a memory protection block using the Read Status command.
  /// @param blockNum Block number to to read status of.
  /// @param[out] protection Receives protection status read from device.
  MaximInterface_EXPORT error_code
  readBlockProtection(int blockNum, BlockProtection & protection) const;

  /// @brief Write memory segment using the Authenticated Write Memory command.
  /// @param pageNum Page number for write operation.
  /// @param segmentNum Segment number within page for write operation.
  /// @param[in] newData New data to write to the segment.
  /// @param[in] mac Write MAC computed for this operation.
  MaximInterface_EXPORT error_code
  writeAuthSegment(int pageNum, int segmentNum, Segment::const_span newData,
                   Sha256::Hash::const_span mac);

  /// @brief Continue an in-progress Authenticated Write Memory command.
  /// @param[in] newData New data to write to the segment.
  /// @param[in] mac Write MAC computed for this operation.
  MaximInterface_EXPORT error_code continueWriteAuthSegment(
      Segment::const_span newData, Sha256::Hash::const_span mac);

  /// @brief
  /// Read the status of all memory protection blocks using the Read Status command.
  /// @param[out] protection Receives protection statuses read from device.
  MaximInterface_EXPORT error_code readAllBlockProtection(
      span<BlockProtection, protectionBlocks> protection) const;
};

/// Interface to the DS28E15 authenticator.
class DS28E15 : public DS28EL15 {
public:
  DS28E15(Sleep & sleep, OneWireMaster & master, const SelectRom & selectRom)
      : DS28EL15(sleep, master, selectRom) {}

  /// @brief Perform Load and Lock Secret command on the device.
  /// @note The secret should already be stored in the scratchpad on the device.
  /// @param lock
  /// Prevent further changes to the secret on the device after loading.
  MaximInterface_EXPORT error_code loadSecret(bool lock);

  /// @brief Perform a Compute and Lock Secret command on the device.
  /// @param pageNum Page number to use as the binding data.
  /// @param lock
  /// Prevent further changes to the secret on the device after computing.
  MaximInterface_EXPORT error_code computeSecret(int pageNum, bool lock);
};

/// Interface to the DS28EL22 (low power) authenticator.
class DS28EL22 : public DS28E15_22_25 {
public:
  // DS28E15_22_25 traits
  static const int memoryPages = 8;
  static const int protectionBlocks = 4;

  DS28EL22(Sleep & sleep, OneWireMaster & master, const SelectRom & selectRom)
      : DS28E15_22_25(sleep, master, selectRom) {}

  /// @brief Perform Write Scratchpad operation on the device.
  /// @param[in] data Data to write to the scratchpad.
  MaximInterface_EXPORT error_code writeScratchpad(Scratchpad::const_span data);

  /// @brief Perform a Read Scratchpad operation on the device.
  /// @param[out] data Buffer to read data from the scratchpad into.
  MaximInterface_EXPORT error_code readScratchpad(Scratchpad::span data) const;

  /// @brief
  /// Read the status of a memory protection block using the Read Status command.
  /// @param blockNum Block number to to read status of.
  /// @param[out] protection Receives protection status read from device.
  MaximInterface_EXPORT error_code
  readBlockProtection(int blockNum, BlockProtection & protection) const;

  /// @brief Write memory segment using the Authenticated Write Memory command.
  /// @param pageNum Page number for write operation.
  /// @param segmentNum Segment number within page for write operation.
  /// @param[in] newData New data to write to the segment.
  /// @param[in] mac Write MAC computed for this operation.
  MaximInterface_EXPORT error_code
  writeAuthSegment(int pageNum, int segmentNum, Segment::const_span newData,
                   Sha256::Hash::const_span mac);

  /// @brief Continue an in-progress Authenticated Write Memory command.
  /// @param[in] newData New data to write to the segment.
  /// @param[in] mac Write MAC computed for this operation.
  MaximInterface_EXPORT error_code continueWriteAuthSegment(
      Segment::const_span newData, Sha256::Hash::const_span mac);

  /// @brief
  /// Read the status of all memory protection blocks using the Read Status command.
  /// @param[out] protection Receives protection statuses read from device.
  MaximInterface_EXPORT error_code readAllBlockProtection(
      span<BlockProtection, protectionBlocks> protection) const;
};

/// Interface to the DS28E22 authenticator.
class DS28E22 : public DS28EL22 {
public:
  DS28E22(Sleep & sleep, OneWireMaster & master, const SelectRom & selectRom)
      : DS28EL22(sleep, master, selectRom) {}

  /// @brief Perform Load and Lock Secret command on the device.
  /// @note The secret should already be stored in the scratchpad on the device.
  /// @param lock
  /// Prevent further changes to the secret on the device after loading.
  MaximInterface_EXPORT error_code loadSecret(bool lock);

  /// @brief Perform a Compute and Lock Secret command on the device.
  /// @param pageNum Page number to use as the binding data.
  /// @param lock
  /// Prevent further changes to the secret on the device after computing.
  MaximInterface_EXPORT error_code computeSecret(int pageNum, bool lock);
};

/// Interface to the DS28EL25 (low power) authenticator.
class DS28EL25 : public DS28E15_22_25 {
public:
  // DS28E15_22_25 traits
  static const int memoryPages = 16;
  static const int protectionBlocks = 8;

  DS28EL25(Sleep & sleep, OneWireMaster & master, const SelectRom & selectRom)
      : DS28E15_22_25(sleep, master, selectRom) {}

  /// @brief Perform Write Scratchpad operation on the device.
  /// @param[in] data Data to write to the scratchpad.
  MaximInterface_EXPORT error_code writeScratchpad(Scratchpad::const_span data);

  /// @brief Perform a Read Scratchpad operation on the device.
  /// @param[out] data Buffer to read data from the scratchpad into.
  MaximInterface_EXPORT error_code readScratchpad(Scratchpad::span data) const;

  /// @brief
  /// Read the status of a memory protection block using the Read Status command.
  /// @param blockNum Block number to to read status of.
  /// @param[out] protection Receives protection status read from device.
  MaximInterface_EXPORT error_code
  readBlockProtection(int blockNum, BlockProtection & protection) const;

  /// Write memory segment using the Authenticated Write Memory command.
  /// @param pageNum Page number for write operation.
  /// @param segmentNum Segment number within page for write operation.
  /// @param[in] newData New data to write to the segment.
  /// @param[in] mac Write MAC computed for this operation.
  MaximInterface_EXPORT error_code
  writeAuthSegment(int pageNum, int segmentNum, Segment::const_span newData,
                   Sha256::Hash::const_span mac);

  /// @brief Continue an in-progress Authenticated Write Memory command.
  /// @param[in] newData New data to write to the segment.
  /// @param[in] mac Write MAC computed for this operation.
  MaximInterface_EXPORT error_code continueWriteAuthSegment(
      Segment::const_span newData, Sha256::Hash::const_span mac);

  /// @brief
  /// Read the status of all memory protection blocks using the Read Status command.
  /// @param[out] protection Receives protection statuses read from device.
  MaximInterface_EXPORT error_code readAllBlockProtection(
      span<BlockProtection, protectionBlocks> protection) const;
};

/// Interface to the DS28E25 authenticator.
class DS28E25 : public DS28EL25 {
public:
  DS28E25(Sleep & sleep, OneWireMaster & master, const SelectRom & selectRom)
      : DS28EL25(sleep, master, selectRom) {}

  /// @brief Perform Load and Lock Secret command on the device.
  /// @note The secret should already be stored in the scratchpad on the device.
  /// @param lock Prevent further changes to the secret on the device after loading.
  MaximInterface_EXPORT error_code loadSecret(bool lock);

  /// @brief Perform a Compute and Lock Secret command on the device.
  /// @param pageNum Page number to use as the binding data.
  /// @param lock
  /// Prevent further changes to the secret on the device after computing.
  MaximInterface_EXPORT error_code computeSecret(int pageNum, bool lock);
};

/// Represents the status of a memory protection block.
class DS28E15_22_25::BlockProtection {
public:
  explicit BlockProtection(uint_least8_t status = 0x00) : status(status) {}

  /// Get the byte representation used by the device.
  uint_least8_t statusByte() const { return status; }
  
  /// Set the byte representation used by the device.
  BlockProtection & setStatusByte(uint_least8_t status) {
    this->status = status;
    return *this;
  }

  /// Get the Block Number which is indexed from zero.
  int blockNum() const { return (status & blockNumMask); }
  
  /// Set the Block Number which is indexed from zero.
  MaximInterface_EXPORT BlockProtection & setBlockNum(int blockNum);

  /// @brief Get the Read Protection status.
  /// @returns True if Read Protection is enabled.
  bool readProtection() const {
    return ((status & readProtectionMask) == readProtectionMask);
  }
  
  /// Set the Read Protection status.
  MaximInterface_EXPORT BlockProtection &
  setReadProtection(bool readProtection);

  /// @brief Get the Write Protection status.
  /// @returns True if Write Protection is enabled.
  bool writeProtection() const {
    return ((status & writeProtectionMask) == writeProtectionMask);
  }
  
  /// Set the Write Protection status.
  MaximInterface_EXPORT BlockProtection &
  setWriteProtection(bool writeProtection);

  /// @brief Get the EEPROM Emulation Mode status.
  /// @returns True if EEPROM Emulation Mode is enabled.
  bool eepromEmulation() const {
    return ((status & eepromEmulationMask) == eepromEmulationMask);
  }
  
  /// Set the EEPROM Emulation Mode status.
  MaximInterface_EXPORT BlockProtection &
  setEepromEmulation(bool eepromEmulation);

  /// @brief Get the Authentication Protection status.
  /// @returns True if Authentication Protection is enabled.
  bool authProtection() const {
    return ((status & authProtectionMask) == authProtectionMask);
  }
  
  /// Set the Authentication Protection status.
  MaximInterface_EXPORT BlockProtection &
  setAuthProtection(bool authProtection);

  /// @brief Check if no protection options are enabled.
  /// @returns True if no protection options are enabled.
  MaximInterface_EXPORT bool noProtection() const;

private:
  static const unsigned int readProtectionMask = 0x80,
                            writeProtectionMask = 0x40,
                            eepromEmulationMask = 0x20,
                            authProtectionMask = 0x10,
                            blockNumMask = 0x0F;
  uint_least8_t status;
};

inline bool operator==(DS28E15_22_25::BlockProtection lhs,
                       DS28E15_22_25::BlockProtection rhs) {
  return lhs.statusByte() == rhs.statusByte();
}

inline bool operator!=(DS28E15_22_25::BlockProtection lhs,
                       DS28E15_22_25::BlockProtection rhs) {
  return !operator==(lhs, rhs);
}

/// Format data to hash for an Authenticated Write to a memory segment.
class DS28E15_22_25::SegmentWriteMacData {
public:
  SegmentWriteMacData() : result_() {}

  /// Formatted data result.
  Sha256::WriteMacData::const_span result() const { return result_; }

  /// @name ROM ID
  /// @brief 1-Wire ROM ID of the device.
  /// @{

  /// Get mutable ROM ID.
  RomId::span romId() {
    return make_span(result_).subspan<romIdIdx, RomId::size>();
  }
  
  /// Get immutable ROM ID.
  RomId::const_span romId() const {
    return const_cast<SegmentWriteMacData &>(*this).romId();
  }
  
  /// Set ROM ID.
  SegmentWriteMacData & setRomId(RomId::const_span romId) {
    copy(romId, this->romId());
    return *this;
  }
  
  /// @}

  /// @name MAN ID
  /// @brief Manufacturer ID of the device.
  /// @{
  
  /// Get mutable MAN ID.
  ManId::span manId() {
    return make_span(result_).subspan<manIdIdx, ManId::size>();
  }
  
  /// Get immutable MAN ID.
  ManId::const_span manId() const {
    return const_cast<SegmentWriteMacData &>(*this).manId();
  }
  
  /// Set MAN ID.
  SegmentWriteMacData & setManId(ManId::const_span manId) {
    copy(manId, this->manId());
    return *this;
  }
  
  /// @}

  /// @name Page number
  /// @brief Page number for write operation.
  /// @{

  /// Get page number.
  int pageNum() const { return result_[pageNumIdx]; }
  
  /// Set page number.
  SegmentWriteMacData & setPageNum(int pageNum) {
    result_[pageNumIdx] = pageNum;
    return *this;
  }
  
  /// @}

  /// @name Segment number
  /// @brief Segment number within page for write operation.
  /// @{
  
  /// Get segment number.
  int segmentNum() const { return result_[segmentNumIdx]; }
  
  /// Set segment number.
  SegmentWriteMacData & setSegmentNum(int segmentNum) {
    result_[segmentNumIdx] = segmentNum;
    return *this;
  }
  
  /// @}

  /// @name Old data
  /// @brief Existing data contained in the segment.
  /// @{
  
  /// Get mutable old data.
  Segment::span oldData() {
    return make_span(result_).subspan<oldDataIdx, Segment::size>();
  }
  
  /// Get immutable old data.
  Segment::const_span oldData() const {
    return const_cast<SegmentWriteMacData &>(*this).oldData();
  }
  
  /// Set old data.
  SegmentWriteMacData & setOldData(Segment::const_span oldData) {
    copy(oldData, this->oldData());
    return *this;
  }
  
  /// @}

  /// @name New data
  /// @brief New data to write to the segment.
  /// @{
  
  /// Get mutable new data.
  Segment::span newData() {
    return make_span(result_).subspan<newDataIdx, Segment::size>();
  }
  
  /// Get immutable new data.
  Segment::const_span newData() const {
    return const_cast<SegmentWriteMacData &>(*this).newData();
  }
  
  /// Set new data.
  SegmentWriteMacData & setNewData(Segment::const_span newData) {
    copy(newData, this->newData());
    return *this;
  }
  
  /// @}

private:
  typedef Sha256::WriteMacData::span::index_type index;

  static const index romIdIdx = 0;
  static const index manIdIdx = romIdIdx + RomId::size;
  static const index pageNumIdx = manIdIdx + ManId::size;
  static const index segmentNumIdx = pageNumIdx + 1;
  static const index oldDataIdx = segmentNumIdx + 1;
  static const index newDataIdx = oldDataIdx + Segment::size;

  Sha256::WriteMacData::array result_;
};

/// Format data to hash for an Authenticated Write to a memory protection block.
class DS28E15_22_25::ProtectionWriteMacData {
public:
  MaximInterface_EXPORT ProtectionWriteMacData();

  /// Formatted data result.
  Sha256::WriteMacData::const_span result() const { return result_; }

  /// @name ROM ID
  /// @brief 1-Wire ROM ID of the device.
  /// @{
  
  /// Get mutable ROM ID.
  RomId::span romId() {
    return make_span(result_).subspan<romIdIdx, RomId::size>();
  }
  
  /// Get immutable ROM ID.
  RomId::const_span romId() const {
    return const_cast<ProtectionWriteMacData &>(*this).romId();
  }
  
  /// Set ROM ID.
  ProtectionWriteMacData & setRomId(RomId::const_span romId) {
    copy(romId, this->romId());
    return *this;
  }
  
  /// @}

  /// @name MAN ID
  /// @brief Manufacturer ID of the device.
  /// @{
  
  /// Get mutable MAN ID.
  ManId::span manId() {
    return make_span(result_).subspan<manIdIdx, ManId::size>();
  }
  
  /// Get immutable MAN ID.
  ManId::const_span manId() const {
    return const_cast<ProtectionWriteMacData &>(*this).manId();
  }
  
  /// Set MAN ID.
  ProtectionWriteMacData & setManId(ManId::const_span manId) {
    copy(manId, this->manId());
    return *this;
  }
  
  /// @}

  /// @name Old protection
  /// @brief Existing protection status in device.
  /// @{
  
  /// Get old protection.
  BlockProtection oldProtection() const { return oldProtection_; }
  
  /// Set old protection.
  MaximInterface_EXPORT ProtectionWriteMacData &
  setOldProtection(BlockProtection oldProtection);
  
  /// @}

  /// @name New protection
  /// @brief New protection status to write.
  /// @{
  
  /// Get new protection.
  BlockProtection newProtection() const { return newProtection_; }
  
  /// Set new protection.
  MaximInterface_EXPORT ProtectionWriteMacData &
  setNewProtection(BlockProtection newProtection);
  
  /// @}

private:
  typedef Sha256::WriteMacData::span::index_type index;

  static const index romIdIdx = 0;
  static const index manIdIdx = romIdIdx + RomId::size;
  static const index blockNumIdx = manIdIdx + ManId::size;
  static const index oldProtectionIdx = blockNumIdx + 2;
  static const index newProtectionIdx = oldProtectionIdx + 4;

  Sha256::WriteMacData::array result_;
  BlockProtection oldProtection_;
  BlockProtection newProtection_;
};

/// @brief
/// Format data to hash for device authentication or computing the next secret
/// from the existing secret.
class DS28E15_22_25::AuthenticationData {
public:
  AuthenticationData() : result_() {}

  /// Formatted data result.
  Sha256::AuthenticationData::const_span result() const { return result_; }

  /// @name Page
  /// @brief Data from a device memory page.
  /// @{
  
  /// Get mutable page.
  Page::span page() {
    return make_span(result_).subspan<pageIdx, Page::size>();
  }
  
  /// Get immutable page.
  Page::const_span page() const {
    return const_cast<AuthenticationData &>(*this).page();
  }
  
  /// Set page.
  AuthenticationData & setPage(Page::const_span page) {
    copy(page, this->page());
    return *this;
  }
  
  /// @}

  /// @name Scratchpad
  /// @brief
  /// Data from device scratchpad used as a random challenge in device
  /// authentication and a partial secret in secret computation.
  /// @{

  /// Get mutable scratchpad.
  Scratchpad::span scratchpad() {
    return make_span(result_).subspan<scratchpadIdx, Scratchpad::size>();
  }
  
  /// Get immutable scratchpad.
  Scratchpad::const_span scratchpad() const {
    return const_cast<AuthenticationData &>(*this).scratchpad();
  }
  
  /// Set scratchpad.
  AuthenticationData & setScratchpad(Scratchpad::const_span scratchpad) {
    copy(scratchpad, this->scratchpad());
    return *this;
  }
  
  /// @}

  /// @name ROM ID
  /// @brief 1-Wire ROM ID of the device.
  /// @{
  
  /// Get mutable ROM ID.
  RomId::span romId() {
    return make_span(result_).subspan<romIdIdx, RomId::size>();
  }
  
  /// Get immutable ROM ID.
  RomId::const_span romId() const {
    return const_cast<AuthenticationData &>(*this).romId();
  }
  
  /// Set ROM ID.
  AuthenticationData & setRomId(RomId::const_span romId) {
    copy(romId, this->romId());
    return *this;
  }
  
  /// Set ROM ID for use in anonymous mode.
  MaximInterface_EXPORT AuthenticationData & setAnonymousRomId();
  
  /// @}

  /// @name MAN ID
  /// @brief Manufacturer ID of the device.
  /// @{
  
  /// Get mutable MAN ID.
  ManId::span manId() {
    return make_span(result_).subspan<manIdIdx, ManId::size>();
  }
  
  /// Get immutable MAN ID.
  ManId::const_span manId() const {
    return const_cast<AuthenticationData &>(*this).manId();
  }
  
  /// Set MAN ID.
  AuthenticationData & setManId(ManId::const_span manId) {
    copy(manId, this->manId());
    return *this;
  }
  
  /// @}

  /// @name Page number
  /// @brief Number of the page to use data from.
  /// @{
  
  /// Get page number.
  int pageNum() const { return result_[pageNumIdx]; }
  
  /// Set page number.
  AuthenticationData & setPageNum(int pageNum) {
    result_[pageNumIdx] = pageNum;
    return *this;
  }
  
  /// @}

private:
  typedef Sha256::AuthenticationData::span::index_type index;

  static const index pageIdx = 0;
  static const index scratchpadIdx = pageIdx + Page::size;
  static const index romIdIdx = scratchpadIdx + Scratchpad::size;
  static const index manIdIdx = romIdIdx + RomId::size;
  static const index pageNumIdx = manIdIdx + ManId::size;

  Sha256::AuthenticationData::array result_;
};

} // namespace MaximInterface

#endif
