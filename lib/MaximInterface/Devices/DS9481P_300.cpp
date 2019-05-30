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
#include "DS9481P_300.hpp"

namespace MaximInterface {

DS9481P_300::DS9481P_300(Sleep & sleep, SerialPort & serialPort)
    : serialPort(&serialPort), currentBus(OneWire), ds2480b(sleep, serialPort),
      oneWireMaster_(*this), ds9400(serialPort), i2cMaster_(*this) {}

error_code DS9481P_300::connect(const std::string & portName) {
  error_code result = serialPort->connect(portName);
  if (!result) {
    result = selectOneWire();
    if (result) {
      serialPort->disconnect();
    } else {
      currentBus = OneWire;
    }
  }
  return result;
}

error_code DS9481P_300::disconnect() {
  return serialPort->disconnect();
}

bool DS9481P_300::connected() const {
  return serialPort->connected();
}

std::string DS9481P_300::portName() const {
  return serialPort->portName();
}

error_code DS9481P_300::selectOneWire() {
  // Escape DS9400 mode.
  error_code result = ds9400.escape();
  if (!result) {
    result = ds2480b.initialize();
  }
  return result;
}

error_code DS9481P_300::selectBus(Bus newBus) {
  error_code result;
  if (currentBus != newBus) {
    switch (currentBus) {
    case OneWire: // Next bus I2C.
      // Escape DS2480 Mode.
      result = ds2480b.escape();
      if (!result) {
        // Wait for awake notification.
        result = ds9400.waitAwake();
      }
      break;

    case I2C: // Next bus OneWire.
      result = selectOneWire();
      break;
    }
    if (!result) {
      currentBus = newBus;
    }
  }
  return result;
}

error_code DS9481P_300::OneWireMasterImpl::reset() {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::reset();
  }
  return result;
}

error_code DS9481P_300::OneWireMasterImpl::touchBitSetLevel(bool & sendRecvBit,
                                                            Level afterLevel) {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::touchBitSetLevel(sendRecvBit, afterLevel);
  }
  return result;
}

error_code
DS9481P_300::OneWireMasterImpl::writeByteSetLevel(uint_least8_t sendByte,
                                                  Level afterLevel) {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::writeByteSetLevel(sendByte, afterLevel);
  }
  return result;
}

error_code
DS9481P_300::OneWireMasterImpl::readByteSetLevel(uint_least8_t & recvByte,
                                                 Level afterLevel) {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::readByteSetLevel(recvByte, afterLevel);
  }
  return result;
}

error_code
DS9481P_300::OneWireMasterImpl::writeBlock(span<const uint_least8_t> sendBuf) {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::writeBlock(sendBuf);
  }
  return result;
}

error_code
DS9481P_300::OneWireMasterImpl::readBlock(span<uint_least8_t> recvBuf) {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::readBlock(recvBuf);
  }
  return result;
}

error_code DS9481P_300::OneWireMasterImpl::setSpeed(Speed newSpeed) {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::setSpeed(newSpeed);
  }
  return result;
}

error_code DS9481P_300::OneWireMasterImpl::setLevel(Level newLevel) {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::setLevel(newLevel);
  }
  return result;
}

error_code DS9481P_300::OneWireMasterImpl::triplet(TripletData & data) {
  error_code result = parent->selectBus(OneWire);
  if (!result) {
    result = OneWireMasterDecorator::triplet(data);
  }
  return result;
}

error_code DS9481P_300::I2CMasterImpl::start(uint_least8_t address) {
  error_code result = parent->selectBus(I2C);
  if (!result) {
    result = I2CMasterDecorator::start(address);
  }
  return result;
}

error_code DS9481P_300::I2CMasterImpl::stop() {
  error_code result = parent->selectBus(I2C);
  if (!result) {
    result = I2CMasterDecorator::stop();
  }
  return result;
}

error_code DS9481P_300::I2CMasterImpl::writeByte(uint_least8_t data) {
  error_code result = parent->selectBus(I2C);
  if (!result) {
    result = I2CMasterDecorator::writeByte(data);
  }
  return result;
}

error_code
DS9481P_300::I2CMasterImpl::writeBlock(span<const uint_least8_t> data) {
  error_code result = parent->selectBus(I2C);
  if (!result) {
    result = I2CMasterDecorator::writeBlock(data);
  }
  return result;
}

error_code DS9481P_300::I2CMasterImpl::writePacketImpl(
    uint_least8_t address, span<const uint_least8_t> data, bool sendStop) {
  error_code result = parent->selectBus(I2C);
  if (!result) {
    result = I2CMasterDecorator::writePacketImpl(address, data, sendStop);
  }
  return result;
}

error_code DS9481P_300::I2CMasterImpl::readByte(AckStatus status,
                                                uint_least8_t & data) {
  error_code result = parent->selectBus(I2C);
  if (!result) {
    result = I2CMasterDecorator::readByte(status, data);
  }
  return result;
}

error_code DS9481P_300::I2CMasterImpl::readBlock(AckStatus status,
                                                 span<uint_least8_t> data) {
  error_code result = parent->selectBus(I2C);
  if (!result) {
    result = I2CMasterDecorator::readBlock(status, data);
  }
  return result;
}

error_code DS9481P_300::I2CMasterImpl::readPacketImpl(uint_least8_t address,
                                                      span<uint_least8_t> data,
                                                      bool sendStop) {
  error_code result = parent->selectBus(I2C);
  if (!result) {
    result = I2CMasterDecorator::readPacketImpl(address, data, sendStop);
  }
  return result;
}

error_code DS9481P_300::DS2480BWithEscape::escape() {
  return sendCommand(0xE5);
}

error_code DS9481P_300::DS9400WithEscape::escape() {
  return configure('O');
}

} // namespace MaximInterface
