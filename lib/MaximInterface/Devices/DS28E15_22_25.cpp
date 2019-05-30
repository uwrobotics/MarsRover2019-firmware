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

#include <algorithm>
#include <MaximInterface/Utilities/crc.hpp>
#include <MaximInterface/Utilities/Error.hpp>
#include "DS28E15_22_25.hpp"

namespace MaximInterface {

using namespace Sha256;
using std::copy;

static const int shaComputationDelayMs = 3;
static const int eepromWriteDelayMs = 10;
static inline int secretEepromWriteDelayMs(bool lowPower) {
  return lowPower ? 200 : 100;
}

static const int ds28e22_25_pagesPerBlock = 2;

static error_code
writeDataWithCrc(OneWireMaster & master, span<const uint_least8_t> data,
                 OneWireMaster::Level level = OneWireMaster::NormalLevel,
                 uint_fast16_t crcStart = 0) {
  error_code result = master.writeBlock(data);
  if (result) {
    return result;
  }
  uint_least8_t response[2];
  result = master.readByte(response[0]);
  if (result) {
    return result;
  }
  result = master.readByteSetLevel(response[1], level);
  if (result) {
    return result;
  }
  if (calculateCrc16(response, calculateCrc16(data, crcStart)) != 0xB001) {
    result = make_error_code(DS28E15_22_25::CrcError);
  }
  return result;
}

const int DS28E15_22_25::segmentsPerPage;

DS28E15_22_25::AuthenticationData &
DS28E15_22_25::AuthenticationData::setAnonymousRomId() {
  std::fill(romId().begin(), romId().end(), 0xFF);
  return *this;
}

error_code
DS28E15_22_25::writeCommandWithCrc(Command command, uint_least8_t parameter,
                                   OneWireMaster::Level level) const {
  error_code result = selectRom(*master);
  if (!result) {
    const uint_least8_t data[] = {static_cast<uint_least8_t>(command),
                                  parameter};
    result = writeDataWithCrc(*master, data);
  }
  return result;
}

static error_code readDataWithCrc(OneWireMaster & master,
                                  span<uint_least8_t> data) {
  error_code result = master.readBlock(data);
  if (result) {
    return result;
  }
  uint_least8_t response[2];
  result = master.readBlock(response);
  if (result) {
    return result;
  }
  if (calculateCrc16(response, calculateCrc16(data)) != 0xB001) {
    result = make_error_code(DS28E15_22_25::CrcError);
  }
  return result;
}

static error_code readCsByte(OneWireMaster & master) {
  uint_least8_t response;
  error_code result = master.readByte(response);
  if (result) {
    return result;
  }
  if (response != 0xAA) {
    result = make_error_code(DS28E15_22_25::OperationFailure);
  }
  return result;
}

static error_code releaseSequence(OneWireMaster & master, Sleep & sleep,
                                  int delayTimeMs) {
  error_code result = master.writeBytePower(0xAA);
  if (result) {
    return result;
  }
  sleep(delayTimeMs);
  result = master.setLevel(OneWireMaster::NormalLevel);
  if (result) {
    return result;
  }
  return readCsByte(master);
}

DS28E15_22_25::BlockProtection &
DS28E15_22_25::BlockProtection::setBlockNum(int blockNum) {
  status &= ~blockNumMask;
  status |= (blockNum & blockNumMask);
  return *this;
}

bool DS28E15_22_25::BlockProtection::noProtection() const {
  return !readProtection() && !writeProtection() && !eepromEmulation() &&
         !authProtection();
}

DS28E15_22_25::BlockProtection &
DS28E15_22_25::BlockProtection::setReadProtection(bool readProtection) {
  if (readProtection) {
    status |= readProtectionMask;
  } else {
    status &= ~readProtectionMask;
  }
  return *this;
}

DS28E15_22_25::BlockProtection &
DS28E15_22_25::BlockProtection::setWriteProtection(bool writeProtection) {
  if (writeProtection) {
    status |= writeProtectionMask;
  } else {
    status &= ~writeProtectionMask;
  }
  return *this;
}

DS28E15_22_25::BlockProtection &
DS28E15_22_25::BlockProtection::setEepromEmulation(bool eepromEmulation) {
  if (eepromEmulation) {
    status |= eepromEmulationMask;
  } else {
    status &= ~eepromEmulationMask;
  }
  return *this;
}

DS28E15_22_25::BlockProtection &
DS28E15_22_25::BlockProtection::setAuthProtection(bool authProtection) {
  if (authProtection) {
    status |= authProtectionMask;
  } else {
    status &= ~authProtectionMask;
  }
  return *this;
}

DS28E15_22_25::ProtectionWriteMacData::ProtectionWriteMacData()
    : result_(), oldProtection_(), newProtection_() {
  setOldProtection(oldProtection_);
  setNewProtection(newProtection_);
}

DS28E15_22_25::ProtectionWriteMacData &
DS28E15_22_25::ProtectionWriteMacData::setOldProtection(
    BlockProtection oldProtection) {
  result_[oldProtectionIdx] = oldProtection.authProtection() ? 1 : 0;
  result_[oldProtectionIdx + 1] = oldProtection.eepromEmulation() ? 1 : 0;
  result_[oldProtectionIdx + 2] = oldProtection.writeProtection() ? 1 : 0;
  result_[oldProtectionIdx + 3] = oldProtection.readProtection() ? 1 : 0;
  oldProtection_ = oldProtection;
  return *this;
}

DS28E15_22_25::ProtectionWriteMacData &
DS28E15_22_25::ProtectionWriteMacData::setNewProtection(
    BlockProtection newProtection) {
  result_[blockNumIdx] = newProtection.blockNum();
  result_[newProtectionIdx] = newProtection.authProtection() ? 1 : 0;
  result_[newProtectionIdx + 1] = newProtection.eepromEmulation() ? 1 : 0;
  result_[newProtectionIdx + 2] = newProtection.writeProtection() ? 1 : 0;
  result_[newProtectionIdx + 3] = newProtection.readProtection() ? 1 : 0;
  newProtection_ = newProtection;
  return *this;
}

error_code
DS28E15_22_25::writeAuthBlockProtection(BlockProtection newProtection,
                                        Hash::const_span mac) {
  error_code result =
      writeCommandWithCrc(AuthWriteBlockProtection, newProtection.statusByte(),
                          OneWireMaster::StrongLevel);
  if (result) {
    return result;
  }

  sleep->invoke(shaComputationDelayMs);
  result = master->setLevel(OneWireMaster::NormalLevel);
  if (result) {
    return result;
  }

  result = writeDataWithCrc(*master, mac);
  if (result) {
    return result;
  }

  result = readCsByte(*master);
  if (result) {
    return result;
  }

  result = releaseSequence(*master, *sleep, eepromWriteDelayMs);
  return result;
}

error_code DS28E15_22_25::writeBlockProtection(BlockProtection protection) {
  error_code result =
      writeCommandWithCrc(WriteBlockProtection, protection.statusByte());
  if (result) {
    return result;
  }

  result = releaseSequence(*master, *sleep, eepromWriteDelayMs);
  return result;
}

error_code DS28E15_22_25::doReadBlockProtection(int blockNum,
                                                BlockProtection & protection,
                                                Variant variant) const {
  uint_least8_t buffer = blockNum;
  if (variant == DS28E22 || variant == DS28E25) {
    buffer *= ds28e22_25_pagesPerBlock;
  }
  error_code result = writeCommandWithCrc(ReadStatus, buffer);
  if (!result) {
    result = master->readByte(buffer);
    if (!result) {
      protection.setStatusByte(buffer);
    }
  }
  return result;
}

error_code DS28E15_22_25::computeReadPageMac(int page_num, bool anon,
                                             Hash::span mac) const {
  error_code result =
      writeCommandWithCrc(ComputePageMac, (anon ? 0xE0 : 0x00) | page_num,
                          OneWireMaster::StrongLevel);
  if (result) {
    return result;
  }

  sleep->invoke(shaComputationDelayMs * 2);
  result = master->setLevel(OneWireMaster::NormalLevel);
  if (result) {
    return result;
  }

  result = readCsByte(*master);
  if (result) {
    return result;
  }

  result = readDataWithCrc(*master, mac);
  return result;
}

error_code DS28E15_22_25::doComputeSecret(int page_num, bool lock,
                                          bool lowPower) {
  error_code result = writeCommandWithCrc(ComputeAndLockSecret,
                                          lock ? (0xE0 | page_num) : page_num);
  if (result) {
    return result;
  }

  result = releaseSequence(*master, *sleep,
                           shaComputationDelayMs * 2 +
                               secretEepromWriteDelayMs(lowPower));
  return result;
}

error_code DS28E15_22_25::doWriteScratchpad(Scratchpad::const_span data,
                                            Variant variant) {
  const uint_least8_t parameter =
      (variant == DS28E22 || variant == DS28E25) ? 0x20 : 0x00;
  error_code result = writeCommandWithCrc(ReadWriteScratchpad, parameter);
  if (result) {
    return result;
  }

  result = writeDataWithCrc(*master, data);
  return result;
}

error_code DS28E15_22_25::doReadScratchpad(Scratchpad::span data,
                                           Variant variant) const {
  const uint_least8_t parameter =
      (variant == DS28E22 || variant == DS28E25) ? 0x2F : 0x0F;
  error_code result = writeCommandWithCrc(ReadWriteScratchpad, parameter);
  if (result) {
    return result;
  }

  result = readDataWithCrc(*master, data);
  return result;
}

error_code DS28E15_22_25::doLoadSecret(bool lock, bool lowPower) {
  error_code result =
      writeCommandWithCrc(LoadAndLockSecret, lock ? 0xE0 : 0x00);
  if (result) {
    return result;
  }

  result = releaseSequence(*master, *sleep, secretEepromWriteDelayMs(lowPower));
  return result;
}

error_code DS28E15_22_25::readPage(int page, Page::span rdbuf) const {
  error_code result = writeCommandWithCrc(ReadMemory, page);
  if (result) {
    return result;
  }

  result = continueReadPage(rdbuf);
  return result;
}

error_code DS28E15_22_25::continueReadPage(Page::span rdbuf) const {
  return readDataWithCrc(*master, rdbuf);
}

error_code DS28E15_22_25::doWriteAuthSegment(Segment::const_span newData,
                                             Hash::const_span mac,
                                             Variant variant, bool continuing) {
  // CRC gets calculated with CS byte when continuing on DS28E22 and DS28E25.
  const uint_fast16_t crcStart =
      ((variant == DS28E22 || variant == DS28E25) && continuing)
          ? calculateCrc16(0xAA)
          : 0;
  error_code result =
      writeDataWithCrc(*master, newData, OneWireMaster::StrongLevel, crcStart);
  if (result) {
    return result;
  }

  sleep->invoke(shaComputationDelayMs);
  result = master->setLevel(OneWireMaster::NormalLevel);
  if (result) {
    return result;
  }

  result = writeDataWithCrc(*master, mac);
  if (result) {
    return result;
  }

  result = readCsByte(*master);
  if (result) {
    return result;
  }

  result = releaseSequence(*master, *sleep, eepromWriteDelayMs);
  return result;
}

error_code DS28E15_22_25::doWriteAuthSegment(int pageNum, int segmentNum,
                                             Segment::const_span newData,
                                             Hash::const_span mac,
                                             Variant variant) {
  error_code result =
      writeCommandWithCrc(AuthWriteMemory, (segmentNum << 5) | pageNum);
  if (result) {
    return result;
  }

  result = doWriteAuthSegment(newData, mac, variant, false);
  return result;
}

error_code DS28E15_22_25::doContinueWriteAuthSegment(
    Segment::const_span newData, Hash::const_span mac, Variant variant) {
  return doWriteAuthSegment(newData, mac, variant, true);
}

error_code DS28E15_22_25::readSegment(int page, int segment,
                                      Segment::span data) const {
  error_code result = writeCommandWithCrc(ReadMemory, (segment << 5) | page);
  if (result) {
    return result;
  }

  result = continueReadSegment(data);
  return result;
}

error_code DS28E15_22_25::continueReadSegment(Segment::span data) const {
  return master->readBlock(data);
}

error_code DS28E15_22_25::writeSegment(int page, int block,
                                       Segment::const_span data) {
  error_code result = writeCommandWithCrc(WriteMemory, (block << 5) | page);
  if (result) {
    return result;
  }

  result = continueWriteSegment(data);
  return result;
}

error_code DS28E15_22_25::continueWriteSegment(Segment::const_span data) {
  error_code result = writeDataWithCrc(*master, data);
  if (result) {
    return result;
  }

  result = releaseSequence(*master, *sleep, eepromWriteDelayMs);
  return result;
}

error_code
DS28E15_22_25::doReadAllBlockProtection(span<BlockProtection> protection,
                                        Variant variant) const {
  error_code result = writeCommandWithCrc(ReadStatus, 0);
  if (!result) {
    if (variant == DS28E22 || variant == DS28E25) {
      // Need to read extra data on DS28E22 to get CRC16.
      uint_least8_t buf[DS28E25::memoryPages];
      result = readDataWithCrc(*master, buf);
      if (!result) {
        const int blocks = ((variant == DS28E22) ? DS28E22::memoryPages
                                                 : DS28E25::memoryPages) /
                           ds28e22_25_pagesPerBlock;
        for (span<BlockProtection>::index_type i = 0;
             i < std::min<span<BlockProtection>::index_type>(protection.size(),
                                                             blocks);
             ++i) {
          protection[i].setStatusByte(
              (buf[i * ds28e22_25_pagesPerBlock] & 0xF0) | // Upper nibble
              ((buf[i * ds28e22_25_pagesPerBlock] & 0x0F) /
               ds28e22_25_pagesPerBlock)); // Lower nibble
        }
      }
    } else { // DS28E15
      uint_least8_t buf[DS28E15::protectionBlocks];
      result = readDataWithCrc(*master, buf);
      if (!result) {
        for (span<BlockProtection>::index_type i = 0;
             i < std::min<span<BlockProtection>::index_type>(
                     protection.size(), DS28E15::protectionBlocks);
             ++i) {
          protection[i].setStatusByte(buf[i]);
        }
      }
    }
  }
  return result;
}

const error_category & DS28E15_22_25::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "DS28E15_22_25"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case CrcError:
        return "CRC Error";

      case OperationFailure:
        return "Operation Failure";

      default:
        return defaultErrorMessage(condition);
      }
    }
  } instance;
  return instance;
}

error_code DS28E15_22_25::loadSecret(bool lock) {
  // Use worst-case low power timing if the device type is not known.
  return doLoadSecret(lock, true);
}

error_code DS28E15_22_25::computeSecret(int pageNum, bool lock) {
  // Use worst-case low power timing if the device type is not known.
  return doComputeSecret(pageNum, lock, true);
}

error_code DS28E15_22_25::readPersonality(Personality & personality) const {
  error_code result = writeCommandWithCrc(ReadStatus, 0xE0);
  if (!result) {
    uint_least8_t data[4];
    result = readDataWithCrc(*master, data);
    if (!result) {
      personality.PB1 = data[0];
      personality.PB2 = data[1];
      personality.manId[0] = data[2];
      personality.manId[1] = data[3];
    }
  }
  return result;
}

const int DS28EL15::memoryPages;
const int DS28EL15::protectionBlocks;

error_code DS28EL15::writeScratchpad(Scratchpad::const_span data) {
  return doWriteScratchpad(data, DS28E15);
}

error_code DS28EL15::readScratchpad(Scratchpad::span data) const {
  return doReadScratchpad(data, DS28E15);
}

error_code DS28EL15::readBlockProtection(int blockNum,
                                         BlockProtection & protection) const {
  return doReadBlockProtection(blockNum, protection, DS28E15);
}

error_code DS28EL15::writeAuthSegment(int pageNum, int segmentNum,
                                      Segment::const_span newData,
                                      Hash::const_span mac) {
  return doWriteAuthSegment(pageNum, segmentNum, newData, mac, DS28E15);
}

error_code DS28EL15::continueWriteAuthSegment(Segment::const_span newData,
                                              Hash::const_span mac) {
  return doContinueWriteAuthSegment(newData, mac, DS28E15);
}

error_code DS28EL15::readAllBlockProtection(
    span<BlockProtection, protectionBlocks> protection) const {
  return doReadAllBlockProtection(protection, DS28E15);
}

error_code DS28E15::loadSecret(bool lock) { return doLoadSecret(lock, false); }

error_code DS28E15::computeSecret(int pageNum, bool lock) {
  return doComputeSecret(pageNum, lock, false);
}

const int DS28EL22::memoryPages;
const int DS28EL22::protectionBlocks;

error_code DS28EL22::writeScratchpad(Scratchpad::const_span data) {
  return doWriteScratchpad(data, DS28E22);
}

error_code DS28EL22::readScratchpad(Scratchpad::span data) const {
  return doReadScratchpad(data, DS28E22);
}

error_code DS28EL22::readBlockProtection(int blockNum,
                                         BlockProtection & protection) const {
  return doReadBlockProtection(blockNum, protection, DS28E22);
}

error_code DS28EL22::writeAuthSegment(int pageNum, int segmentNum,
                                      Segment::const_span newData,
                                      Hash::const_span mac) {
  return doWriteAuthSegment(pageNum, segmentNum, newData, mac, DS28E22);
}

error_code DS28EL22::continueWriteAuthSegment(Segment::const_span newData,
                                              Hash::const_span mac) {
  return doContinueWriteAuthSegment(newData, mac, DS28E22);
}

error_code DS28EL22::readAllBlockProtection(
    span<BlockProtection, protectionBlocks> protection) const {
  return doReadAllBlockProtection(protection, DS28E22);
}

error_code DS28E22::loadSecret(bool lock) { return doLoadSecret(lock, false); }

error_code DS28E22::computeSecret(int pageNum, bool lock) {
  return doComputeSecret(pageNum, lock, false);
}

const int DS28EL25::memoryPages;
const int DS28EL25::protectionBlocks;

error_code DS28EL25::writeScratchpad(Scratchpad::const_span data) {
  return doWriteScratchpad(data, DS28E25);
}

error_code DS28EL25::readScratchpad(Scratchpad::span data) const {
  return doReadScratchpad(data, DS28E25);
}

error_code DS28EL25::readBlockProtection(int blockNum,
                                         BlockProtection & protection) const {
  return doReadBlockProtection(blockNum, protection, DS28E25);
}

error_code DS28EL25::writeAuthSegment(int pageNum, int segmentNum,
                                      Segment::const_span newData,
                                      Hash::const_span mac) {
  return doWriteAuthSegment(pageNum, segmentNum, newData, mac, DS28E25);
}

error_code DS28EL25::continueWriteAuthSegment(Segment::const_span newData,
                                              Hash::const_span mac) {
  return doContinueWriteAuthSegment(newData, mac, DS28E25);
}

error_code DS28EL25::readAllBlockProtection(
    span<BlockProtection, protectionBlocks> protection) const {
  return doReadAllBlockProtection(protection, DS28E25);
}

error_code DS28E25::loadSecret(bool lock) { return doLoadSecret(lock, false); }

error_code DS28E25::computeSecret(int pageNum, bool lock) {
  return doComputeSecret(pageNum, lock, false);
}

} // namespace MaximInterface
