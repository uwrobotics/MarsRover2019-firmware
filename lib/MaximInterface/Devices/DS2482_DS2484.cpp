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
#include "DS2482_DS2484.hpp"

namespace MaximInterface {

// Device Status bits
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

error_code DS2482_DS2484::initialize(Config config) {
  error_code result = resetDevice();
  if (result) {
    return result;
  }
  // Write the default configuration setup.
  result = writeConfig(config);
  return result;
}

error_code DS2482_DS2484::resetDevice() {
  // Device Reset
  //   S AD,0 [A] DRST [A] Sr AD,1 [A] [SS] A\ P
  //  [] indicates from slave
  //  SS status byte to read to verify state

  error_code result = sendCommand(0xF0);
  if (result) {
    return result;
  }

  uint_least8_t buf;
  result = readRegister(buf);
  if (result) {
    return result;
  }

  if ((buf & 0xF7) != 0x10) {
    return make_error_code(HardwareError);
  }

  // Do a command to get 1-Wire master reset out of holding state.
  reset();

  return result;
}

error_code DS2482_DS2484::triplet(TripletData & data) {
  // 1-Wire Triplet (Case B)
  //   S AD,0 [A] 1WT [A] SS [A] Sr AD,1 [A] [Status] A [Status] A\ P
  //                                         \--------/
  //                           Repeat until 1WB bit has changed to 0
  //  [] indicates from slave
  //  SS indicates byte containing search direction bit value in msbit

  error_code result = sendCommand(0x78, data.writeBit ? 0x80 : 0x00);
  if (!result) {
    uint_least8_t status;
    result = pollBusy(&status);
    if (!result) {
      data.readBit = ((status & Status_SBR) == Status_SBR);
      data.readBitComplement = ((status & Status_TSB) == Status_TSB);
      data.writeBit = ((status & Status_DIR) == Status_DIR);
    }
  }
  return result;
}

error_code DS2482_DS2484::reset() {
  // 1-Wire reset (Case B)
  //   S AD,0 [A] 1WRS [A] Sr AD,1 [A] [Status] A [Status] A\ P
  //                                   \--------/
  //                       Repeat until 1WB bit has changed to 0
  //  [] indicates from slave

  error_code result = sendCommand(0xB4);
  if (result) {
    return result;
  }

  uint_least8_t buf;
  result = pollBusy(&buf);
  if (result) {
    return result;
  }

  if ((buf & Status_SD) == Status_SD) {
    result = make_error_code(ShortDetectedError);
  } else if ((buf & Status_PPD) != Status_PPD) {
    result = make_error_code(NoSlaveError);
  }

  return result;
}

error_code DS2482_DS2484::touchBitSetLevel(bool & sendRecvBit,
                                           Level afterLevel) {
  // 1-Wire bit (Case B)
  //   S AD,0 [A] 1WSB [A] BB [A] Sr AD,1 [A] [Status] A [Status] A\ P
  //                                          \--------/
  //                           Repeat until 1WB bit has changed to 0
  //  [] indicates from slave
  //  BB indicates byte containing bit value in msbit

  error_code result = configureLevel(afterLevel);
  if (result) {
    return result;
  }

  result = sendCommand(0x87, sendRecvBit ? 0x80 : 0x00);
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

error_code DS2482_DS2484::writeByteSetLevel(uint_least8_t sendByte,
                                            Level afterLevel) {
  // 1-Wire Write Byte (Case B)
  //   S AD,0 [A] 1WWB [A] DD [A] Sr AD,1 [A] [Status] A [Status] A\ P
  //                                          \--------/
  //                             Repeat until 1WB bit has changed to 0
  //  [] indicates from slave
  //  DD data to write

  error_code result = configureLevel(afterLevel);
  if (result) {
    return result;
  }

  result = sendCommand(0xA5, sendByte);
  if (result) {
    return result;
  }

  result = pollBusy();
  return result;
}

error_code DS2482_DS2484::readByteSetLevel(uint_least8_t & recvByte,
                                           Level afterLevel) {
  // 1-Wire Read Bytes (Case C)
  //   S AD,0 [A] 1WRB [A] Sr AD,1 [A] [Status] A [Status] A
  //                                   \--------/
  //                     Repeat until 1WB bit has changed to 0
  //   Sr AD,0 [A] SRP [A] E1 [A] Sr AD,1 [A] DD A\ P
  //
  //  [] indicates from slave
  //  DD data read

  error_code result = configureLevel(afterLevel);
  if (result) {
    return result;
  }

  result = sendCommand(0x96);
  if (result) {
    return result;
  }

  result = pollBusy();
  if (result) {
    return result;
  }

  result = readRegister(0xE1, recvByte);
  return result;
}

error_code DS2482_DS2484::setSpeed(Speed newSpeed) {
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

error_code DS2482_DS2484::setLevel(Level newLevel) {
  if (newLevel == StrongLevel) {
    return make_error_code(InvalidLevelError);
  }

  return configureLevel(newLevel);
}

error_code DS2482_DS2484::writeConfig(Config config) {
  uint_least8_t configBuf =
      ((config.readByte() ^ 0xF) << 4) | config.readByte();
  error_code result = sendCommand(0xD2, configBuf);
  if (!result) {
    result = readRegister(0xC3, configBuf);
  }
  if (!result) {
    if (configBuf != config.readByte()) {
      result = make_error_code(HardwareError);
    }
  }
  if (!result) {
    curConfig = config;
  }
  return result;
}

error_code DS2482_DS2484::readRegister(uint_least8_t reg,
                                       uint_least8_t & buf) const {
  error_code result = sendCommand(0xE1, reg);
  if (!result) {
    result = readRegister(buf);
  }
  return result;
}

error_code DS2482_DS2484::readRegister(uint_least8_t & buf) const {
  return master->readPacket(address_, make_span(&buf, 1));
}

error_code DS2482_DS2484::pollBusy(uint_least8_t * pStatus) {
  const int pollLimit = 200;

  int pollCount = 0;
  uint_least8_t status;
  do {
    error_code result = readRegister(status);
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

error_code DS2482_DS2484::configureLevel(Level level) {
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

error_code DS2482_DS2484::sendCommand(uint_least8_t cmd) const {
  return master->writePacket(address_, make_span(&cmd, 1));
}

error_code DS2482_DS2484::sendCommand(uint_least8_t cmd,
                                      uint_least8_t param) const {
  uint_least8_t buf[] = {cmd, param};
  return master->writePacket(address_, buf);
}

const error_category & DS2482_DS2484::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "DS2482_DS2484"; }

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

error_code DS2482_800::selectChannel(int channel) {
  // Channel Select (Case A)
  //   S AD,0 [A] CHSL [A] CC [A] Sr AD,1 [A] [RR] A\ P
  //  [] indicates from slave
  //  CC channel value
  //  RR channel read back

  uint_least8_t ch;
  uint_least8_t ch_read;
  switch (channel) {
  case 0:
    ch = 0xF0;
    ch_read = 0xB8;
    break;

  case 1:
    ch = 0xE1;
    ch_read = 0xB1;
    break;

  case 2:
    ch = 0xD2;
    ch_read = 0xAA;
    break;

  case 3:
    ch = 0xC3;
    ch_read = 0xA3;
    break;

  case 4:
    ch = 0xB4;
    ch_read = 0x9C;
    break;

  case 5:
    ch = 0xA5;
    ch_read = 0x95;
    break;

  case 6:
    ch = 0x96;
    ch_read = 0x8E;
    break;

  case 7:
    ch = 0x87;
    ch_read = 0x87;
    break;

  default:
    return make_error_code(ArgumentOutOfRangeError);
  };

  error_code result = sendCommand(0xC3, ch);
  if (!result) {
    result = readRegister(ch);
    if (!result) {
      // check for failure due to incorrect read back of channel
      if (ch != ch_read) {
        result = make_error_code(HardwareError);
      }
    }
  }

  return result;
}

error_code DS2484::adjustPort(PortParameter param, int val) {
  if (val < 0 || val > 15) {
    return make_error_code(ArgumentOutOfRangeError);
  }

  error_code result = sendCommand(0xC3, (param << 4) | val);
  if (result) {
    return result;
  }

  uint_least8_t portConfig = val + 1;
  for (int reads = -1; reads < param; ++reads) {
    result = readRegister(portConfig);
    if (result) {
      return result;
    }
  }
  if (val != portConfig) {
    result = make_error_code(HardwareError);
  }

  return result;
}

} // namespace MaximInterface
