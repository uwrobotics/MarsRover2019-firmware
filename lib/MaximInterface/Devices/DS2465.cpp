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

#include <MaximInterface/Utilities/Error.hpp>
#include "DS2465.hpp"

namespace MaximInterface {

using namespace Sha256;

/// Delay required after writing an EEPROM segment.
static const int eepromSegmentWriteDelayMs = 10;
/// Delay required after writing an EEPROM page such as the secret memory.
static const int eepromPageWriteDelayMs = 8 * eepromSegmentWriteDelayMs;
/// Delay required for a SHA computation to complete.
static const int shaComputationDelayMs = 2;

static const uint_least8_t scratchpad = 0x00;
static const uint_least8_t commandReg = 0x60;

static const uint_least8_t owTransmitBlockCmd = 0x69;

/// DS2465 Status bits.
enum StatusBit {
  Status_1WB = 0x01,
  Status_PPD = 0x02,
  Status_SD = 0x04,
  Status_LL = 0x08,
  Status_RST = 0x10,
  Status_SBR = 0x20,
  Status_TSB = 0x40,
  Status_DIR = 0x80
};

static const int maxBlockSize = 63;

const int DS2465::memoryPages;
const int DS2465::segmentsPerPage;

error_code DS2465::initialize(Config config) {
  // reset DS2465
  error_code result = resetDevice();
  if (!result) {
    // write the default configuration setup
    result = writeConfig(config);
  }
  return result;
}

error_code DS2465::computeNextMasterSecret(bool swap, int pageNum,
                                           PageRegion region) {
  error_code result = make_error_code(ArgumentOutOfRangeError);
  if (pageNum >= 0) {
    const uint_least8_t command[] = {
        0x1E, static_cast<uint_least8_t>(swap ? (0xC8 | (pageNum << 4) | region)
                                              : 0xBF)};
    result = writeMemory(commandReg, command);
  }
  return result;
}

error_code DS2465::computeWriteMac(bool regwrite, bool swap, int pageNum,
                                   int segmentNum) const {
  error_code result = make_error_code(ArgumentOutOfRangeError);
  if (pageNum >= 0 && segmentNum >= 0) {
    const uint_least8_t command[] = {
        0x2D, static_cast<uint_least8_t>((regwrite << 7) | (swap << 6) |
                                         (pageNum << 4) | segmentNum)};
    result = writeMemory(commandReg, command);
    if (!result) {
      sleep->invoke(shaComputationDelayMs);
    }
  }
  return result;
}

error_code DS2465::computeAuthMac(bool swap, int pageNum,
                                  PageRegion region) const {
  error_code result = make_error_code(ArgumentOutOfRangeError);
  if (pageNum >= 0) {
    const uint_least8_t command[] = {
        0x3C, static_cast<uint_least8_t>(swap ? (0xC8 | (pageNum << 4) | region)
                                              : 0xBF)};
    result = writeMemory(commandReg, command);
    if (!result) {
      sleep->invoke(shaComputationDelayMs * 2);
    }
  }
  return result;
}

error_code DS2465::computeSlaveSecret(bool swap, int pageNum,
                                      PageRegion region) {
  error_code result = make_error_code(ArgumentOutOfRangeError);
  if (pageNum >= 0) {
    const uint_least8_t command[] = {
        0x4B, static_cast<uint_least8_t>(swap ? (0xC8 | (pageNum << 4) | region)
                                              : 0xBF)};
    result = writeMemory(commandReg, command);
    if (!result) {
      sleep->invoke(shaComputationDelayMs * 2);
    }
  }
  return result;
}

error_code DS2465::readPage(int pageNum, Page::span data) const {
  uint_least8_t addr;
  switch (pageNum) {
  case 0:
    addr = 0x80;
    break;
  case 1:
    addr = 0xA0;
    break;
  default:
    return make_error_code(ArgumentOutOfRangeError);
  }
  return readMemory(addr, data);
}

error_code DS2465::writePage(int pageNum, Page::const_span data) {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = copyScratchpad(false, pageNum, false, 0);
  }
  if (!result) {
    sleep->invoke(eepromPageWriteDelayMs);
  }
  return result;
}

error_code DS2465::writeSegment(int pageNum, int segmentNum,
                                Segment::const_span data) {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = copyScratchpad(false, pageNum, true, segmentNum);
  }
  if (!result) {
    sleep->invoke(eepromSegmentWriteDelayMs);
  }
  return result;
}

error_code DS2465::writeMasterSecret(Hash::const_span masterSecret) {
  error_code result = writeMemory(scratchpad, masterSecret);
  if (!result) {
    result = copyScratchpad(true, 0, false, 0);
  }
  if (!result) {
    sleep->invoke(eepromPageWriteDelayMs);
  }
  return result;
}

error_code DS2465::copyScratchpad(bool destSecret, int pageNum, bool notFull,
                                  int segmentNum) {
  error_code result = make_error_code(ArgumentOutOfRangeError);
  if (pageNum >= 0 && segmentNum >= 0) {
    const uint_least8_t command[] = {
        0x5A,
        static_cast<uint_least8_t>(destSecret ? 0
                                              : (0x80 | (pageNum << 4) |
                                                 (notFull << 3) | segmentNum))};
    result = writeMemory(commandReg, command);
  }
  return result;
}

error_code DS2465::configureLevel(Level level) {
  // Check if supported level
  if (!((level == NormalLevel) || (level == StrongLevel))) {
    return make_error_code(InvalidLevelError);
  }
  // Check if requested level already set
  if (curConfig.getSPU() == (level == StrongLevel)) {
    return error_code();
  }
  // Set the level
  return writeConfig(Config(curConfig).setSPU(level == StrongLevel));
}

error_code DS2465::setLevel(Level newLevel) {
  if (newLevel == StrongLevel) {
    return make_error_code(InvalidLevelError);
  }

  return configureLevel(newLevel);
}

error_code DS2465::setSpeed(Speed newSpeed) {
  // Check if supported speed
  if (!((newSpeed == OverdriveSpeed) || (newSpeed == StandardSpeed))) {
    return make_error_code(InvalidSpeedError);
  }
  // Check if requested speed is already set
  if (curConfig.get1WS() == (newSpeed == OverdriveSpeed)) {
    return error_code();
  }
  // Set the speed
  return writeConfig(Config(curConfig).set1WS(newSpeed == OverdriveSpeed));
}

error_code DS2465::triplet(TripletData & data) {
  // 1-Wire Triplet (Case B)
  //   S AD,0 [A] 1WT [A] SS [A] Sr AD,1 [A] [Status] A [Status] A\ P
  //                                         \--------/
  //                           Repeat until 1WB bit has changed to 0
  //  [] indicates from slave
  //  SS indicates byte containing search direction bit value in msbit

  const uint_least8_t command[] = {
      0x78, static_cast<uint_least8_t>(data.writeBit ? 0x80 : 0x00)};
  error_code result = writeMemory(commandReg, command);
  if (!result) {
    uint_least8_t status;
    result = pollBusy(&status);
    if (!result) {
      // check bit results in status byte
      data.readBit = ((status & Status_SBR) == Status_SBR);
      data.readBitComplement = ((status & Status_TSB) == Status_TSB);
      data.writeBit = ((status & Status_DIR) == Status_DIR);
    }
  }
  return result;
}

error_code DS2465::readBlock(span<uint_least8_t> recvBuf) {
  // 1-Wire Receive Block (Case A)
  //   S AD,0 [A] CommandReg [A] 1WRF [A] PR [A] P
  //  [] indicates from slave
  //  PR indicates byte containing parameter

  error_code result;
  span<uint_least8_t>::index_type recvIdx = 0;
  while (recvIdx < recvBuf.size() && !result) {
    const uint_least8_t command[] = {
        0xE1,
        static_cast<uint_least8_t>(std::min<span<uint_least8_t>::index_type>(
            recvBuf.size() - recvIdx, maxBlockSize))};
    result = writeMemory(commandReg, command);
    if (!result) {
      result = pollBusy();
    }
    if (!result) {
      result = readMemory(scratchpad, recvBuf.subspan(recvIdx, command[1]));
    }
    recvIdx += command[1];
  }
  return result;
}

error_code DS2465::writeBlock(span<const uint_least8_t> sendBuf) {
  error_code result;
  span<const uint_least8_t>::index_type sendIdx = 0;
  while (sendIdx < sendBuf.size() && !result) {
    const uint_least8_t command[] = {
        owTransmitBlockCmd, static_cast<uint_least8_t>(
                                std::min<span<const uint_least8_t>::index_type>(
                                    sendBuf.size() - sendIdx, maxBlockSize))};

    // prefill scratchpad with required data
    result = writeMemory(scratchpad, sendBuf.subspan(sendIdx, command[1]));

    // 1-Wire Transmit Block (Case A)
    //   S AD,0 [A] CommandReg [A] 1WTB [A] PR [A] P
    //  [] indicates from slave
    //  PR indicates byte containing parameter
    if (!result) {
      result = writeMemory(commandReg, command);
    }
    if (!result) {
      result = pollBusy();
    }
    sendIdx += command[1];
  }
  return result;
}

error_code DS2465::writeMacBlock() const {
  // 1-Wire Transmit Block (Case A)
  //   S AD,0 [A] CommandReg [A] 1WTB [A] PR [A] P
  //  [] indicates from slave
  //  PR indicates byte containing parameter

  const uint_least8_t command[] = {owTransmitBlockCmd, 0xFF};
  error_code result = writeMemory(commandReg, command);
  if (!result) {
    result = pollBusy();
  }
  return result;
}

error_code DS2465::readByteSetLevel(uint_least8_t & recvByte,
                                    Level afterLevel) {
  // 1-Wire Read Bytes (Case C)
  //   S AD,0 [A] CommandReg [A] 1WRB [A] Sr AD,1 [A] [Status] A [Status] A
  //                                                  \--------/
  //                     Repeat until 1WB bit has changed to 0
  //   Sr AD,0 [A] SRP [A] E1 [A] Sr AD,1 [A] DD A\ P
  //
  //  [] indicates from slave
  //  DD data read

  error_code result = configureLevel(afterLevel);
  if (result) {
    return result;
  }

  const uint_least8_t command = 0x96;
  result = writeMemory(commandReg, make_span(&command, 1));
  if (result) {
    return result;
  }

  result = pollBusy();
  if (result) {
    return result;
  }

  result = readMemory(0x62, make_span(&recvByte, 1));
  return result;
}

error_code DS2465::writeByteSetLevel(uint_least8_t sendByte, Level afterLevel) {
  // 1-Wire Write Byte (Case B)
  //   S AD,0 [A] CommandReg [A] 1WWB [A] DD [A] Sr AD,1 [A] [Status] A [Status]
  //   A\ P
  //                                                           \--------/
  //                             Repeat until 1WB bit has changed to 0
  //  [] indicates from slave
  //  DD data to write

  error_code result = configureLevel(afterLevel);
  if (result) {
    return result;
  }

  const uint_least8_t command[] = {0xA5, sendByte};
  result = writeMemory(commandReg, command);
  if (result) {
    return result;
  }

  result = pollBusy();
  return result;
}

error_code DS2465::touchBitSetLevel(bool & sendRecvBit, Level afterLevel) {
  // 1-Wire bit (Case B)
  //   S AD,0 [A] CommandReg [A] 1WSB [A] BB [A] Sr AD,1 [A] [Status] A [Status]
  //   A\ P
  //                                                          \--------/
  //                           Repeat until 1WB bit has changed to 0
  //  [] indicates from slave
  //  BB indicates byte containing bit value in msbit

  error_code result = configureLevel(afterLevel);
  if (result) {
    return result;
  }

  const uint_least8_t command[] = {
      0x87, static_cast<uint_least8_t>(sendRecvBit ? 0x80 : 0x00)};
  result = writeMemory(commandReg, command);
  if (result) {
    return result;
  }

  uint_least8_t status;
  result = pollBusy(&status);
  if (!result) {
    sendRecvBit = ((status & Status_SBR) == Status_SBR);
  }
  return result;
}

error_code DS2465::writeMemory(uint_least8_t addr,
                               span<const uint_least8_t> buf) const {
  // Write SRAM (Case A)
  //   S AD,0 [A] VSA [A] DD [A]  P
  //                      \-----/
  //                        Repeat for each data byte
  //  [] indicates from slave
  //  VSA valid SRAM memory address
  //  DD memory data to write

  error_code result = master->start(address_);
  if (result) {
    master->stop();
    return result;
  }
  result = master->writeByte(addr);
  if (result) {
    master->stop();
    return result;
  }
  result = master->writeBlock(buf);
  if (result) {
    master->stop();
    return result;
  }
  result = master->stop();
  return result;
}

error_code DS2465::readMemory(uint_least8_t addr,
                              span<uint_least8_t> buf) const {
  // Read (Case A)
  //   S AD,0 [A] MA [A] Sr AD,1 [A] [DD] A [DD] A\ P
  //                                 \-----/
  //                                   Repeat for each data byte, NAK last byte
  //  [] indicates from slave
  //  MA memory address
  //  DD memory data read

  error_code result = master->start(address_);
  if (result) {
    master->stop();
    return result;
  }
  result = master->writeByte(addr);
  if (result) {
    master->stop();
    return result;
  }
  result = readMemory(buf);
  return result;
}

error_code DS2465::readMemory(span<uint_least8_t> buf) const {
  error_code result = master->start(address_ | 1);
  if (result) {
    master->stop();
    return result;
  }
  result = master->readBlock(I2CMaster::Nack, buf);
  if (result) {
    master->stop();
    return result;
  }
  result = master->stop();
  return result;
}

error_code DS2465::writeConfig(Config config) {
  const uint_least8_t configReg = 0x67;
  uint_least8_t configBuf =
      ((config.readByte() ^ 0xF) << 4) | config.readByte();
  error_code result = writeMemory(configReg, make_span(&configBuf, 1));
  if (!result) {
    result = readMemory(configReg, make_span(&configBuf, 1));
  }
  if (!result) {
    if (configBuf != config.readByte())
      result = make_error_code(HardwareError);
  }
  if (!result) {
    curConfig = config;
  }
  return result;
}

error_code DS2465::writePortParameter(PortParameter param, int val) {
  if (val < 0 || val > 15) {
    return make_error_code(ArgumentOutOfRangeError);
  }

  uint_least8_t addr = 0;
  switch (param) {
  case tRSTL_STD:
  case tRSTL_OD:
    addr = 0x68;
    break;
  case tMSP_STD:
  case tMSP_OD:
    addr = 0x69;
    break;
  case tW0L_STD:
  case tW0L_OD:
    addr = 0x6A;
    break;
  case tREC0:
    addr = 0x6B;
    break;
  case RWPU:
    addr = 0x6C;
    break;
  case tW1L_OD:
    addr = 0x6D;
    break;
  }

  uint_least8_t data;
  error_code result = readMemory(addr, make_span(&data, 1));
  if (result) {
    return result;
  }

  uint_least8_t newData;
  if (param == tRSTL_OD || param == tMSP_OD || param == tW0L_OD) {
    newData = (data & 0x0F) | (val << 4);
  } else {
    newData = (data & 0xF0) | val;
  }

  if (newData != data) {
    result = writeMemory(addr, make_span(&newData, 1));
  }
  return result;
}

error_code DS2465::pollBusy(uint_least8_t * pStatus) const {
  const int pollLimit = 200;

  int pollCount = 0;
  uint_least8_t status;
  do {
    error_code result = readMemory(make_span(&status, 1));
    if (result) {
      return result;
    }
    if (pStatus != NULL) {
      *pStatus = status;
    }
    if (pollCount++ >= pollLimit) {
      return make_error_code(HardwareError);
    }
  } while (status & Status_1WB);

  return error_code();
}

error_code DS2465::reset() {
  // 1-Wire reset (Case B)
  //   S AD,0 [A] CommandReg  [A] 1WRS [A] Sr AD,1 [A] [Status] A [Status] A\ P
  //                                                  \--------/
  //                       Repeat until 1WB bit has changed to 0
  //  [] indicates from slave

  uint_least8_t buf = 0xB4;
  error_code result = writeMemory(commandReg, make_span(&buf, 1));

  if (!result) {
    result = pollBusy(&buf);
  }

  if (!result) {
    if ((buf & Status_SD) == Status_SD) {
      result = make_error_code(ShortDetectedError);
    } else if ((buf & Status_PPD) != Status_PPD) {
      result = make_error_code(NoSlaveError);
    }
  }

  return result;
}

error_code DS2465::resetDevice() {
  // Device Reset
  //   S AD,0 [A] CommandReg [A] 1WMR [A] Sr AD,1 [A] [SS] A\ P
  //  [] indicates from slave
  //  SS status byte to read to verify state

  uint_least8_t buf = 0xF0;
  error_code result = writeMemory(commandReg, make_span(&buf, 1));

  if (!result) {
    result = readMemory(make_span(&buf, 1));
  }

  if (!result) {
    if ((buf & 0xF7) != 0x10) {
      result = make_error_code(HardwareError);
    }
  }

  if (!result) {
    reset(); // do a command to get 1-Wire master reset out of holding state
  }

  return result;
}

error_code
DS2465::computeNextMasterSecret(AuthenticationData::const_span data) {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = computeNextMasterSecret(false, 0, FullPage);
  }
  return result;
}

error_code
DS2465::computeNextMasterSecretWithSwap(AuthenticationData::const_span data,
                                        int pageNum, PageRegion region) {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = computeNextMasterSecret(true, pageNum, region);
  }
  return result;
}

error_code DS2465::computeWriteMac(WriteMacData::const_span data) const {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = computeWriteMac(false, false, 0, 0);
  }
  return result;
}

error_code DS2465::computeWriteMac(WriteMacData::const_span data,
                                   Hash::span mac) const {
  error_code result = computeWriteMac(data);
  if (!result) {
    result = readMemory(mac);
  }
  return result;
}

error_code
DS2465::computeAndTransmitWriteMac(WriteMacData::const_span data) const {
  error_code result = computeWriteMac(data);
  if (!result) {
    result = writeMacBlock();
  }
  return result;
}

error_code DS2465::computeWriteMacWithSwap(WriteMacData::const_span data,
                                           int pageNum, int segmentNum) const {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = computeWriteMac(false, true, pageNum, segmentNum);
  }
  return result;
}

error_code DS2465::computeWriteMacWithSwap(WriteMacData::const_span data,
                                           int pageNum, int segmentNum,
                                           Hash::span mac) const {
  error_code result = computeWriteMacWithSwap(data, pageNum, segmentNum);
  if (!result) {
    result = readMemory(mac);
  }
  return result;
}

error_code
DS2465::computeAndTransmitWriteMacWithSwap(WriteMacData::const_span data,
                                           int pageNum, int segmentNum) const {
  error_code result = computeWriteMacWithSwap(data, pageNum, segmentNum);
  if (!result) {
    result = writeMacBlock();
  }
  return result;
}

error_code DS2465::computeSlaveSecret(AuthenticationData::const_span data) {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = computeSlaveSecret(false, 0, FullPage);
  }
  return result;
}

error_code
DS2465::computeSlaveSecretWithSwap(AuthenticationData::const_span data,
                                   int pageNum, PageRegion region) {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = computeSlaveSecret(true, pageNum, region);
  }
  return result;
}

error_code DS2465::computeAuthMac(AuthenticationData::const_span data) const {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = computeAuthMac(false, 0, FullPage);
  }
  return result;
}

error_code DS2465::computeAuthMac(AuthenticationData::const_span data,
                                  Hash::span mac) const {
  error_code result = computeAuthMac(data);
  if (!result) {
    result = readMemory(mac);
  }
  return result;
}

error_code
DS2465::computeAndTransmitAuthMac(AuthenticationData::const_span data) const {
  error_code result = computeAuthMac(data);
  if (!result) {
    result = writeMacBlock();
  }
  return result;
}

error_code DS2465::computeAuthMacWithSwap(AuthenticationData::const_span data,
                                          int pageNum,
                                          PageRegion region) const {
  error_code result = writeMemory(scratchpad, data);
  if (!result) {
    result = computeAuthMac(true, pageNum, region);
  }
  return result;
}

error_code DS2465::computeAuthMacWithSwap(AuthenticationData::const_span data,
                                          int pageNum, PageRegion region,
                                          Hash::span mac) const {
  error_code result = computeAuthMacWithSwap(data, pageNum, region);
  if (!result) {
    result = readMemory(mac);
  }
  return result;
}

error_code DS2465::computeAndTransmitAuthMacWithSwap(
    AuthenticationData::const_span data, int pageNum, PageRegion region) const {
  error_code result = computeAuthMacWithSwap(data, pageNum, region);
  if (!result) {
    result = writeMacBlock();
  }
  return result;
}

const error_category & DS2465::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "DS2465"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case HardwareError:
        return "Hardware Error";

      case ArgumentOutOfRangeError:
        return "Argument Out of Range Error";

      default:
        return defaultErrorMessage(condition);
      }
    }
  } instance;
  return instance;
}

} // namespace MaximInterface
