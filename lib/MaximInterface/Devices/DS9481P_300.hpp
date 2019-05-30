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

#ifndef MaximInterface_DS9481P_300
#define MaximInterface_DS9481P_300

#include <MaximInterface/Devices/DS2480B.hpp>
#include <MaximInterface/Devices/DS9400.hpp>
#include <MaximInterface/Links/I2CMasterDecorator.hpp>
#include <MaximInterface/Links/OneWireMasterDecorator.hpp>
#include <MaximInterface/Links/SerialPort.hpp>
#include <MaximInterface/Links/Sleep.hpp>
#include <MaximInterface/Utilities/Export.h>

namespace MaximInterface {

/// DS9481P-300 USB to 1-Wire and I2C adapter.
class DS9481P_300 {
public:
  MaximInterface_EXPORT DS9481P_300(Sleep & sleep, SerialPort & serialPort);

  void setSleep(Sleep & sleep) { ds2480b.setSleep(sleep); }
  
  void setSerialPort(SerialPort & serialPort) {
    this->serialPort = &serialPort;
    ds2480b.setUart(serialPort);
    ds9400.setUart(serialPort);
  }

  MaximInterface_EXPORT error_code connect(const std::string & portName);
  
  MaximInterface_EXPORT error_code disconnect();
  
  MaximInterface_EXPORT bool connected() const;
  
  MaximInterface_EXPORT std::string portName() const;

  /// Access the 1-Wire master when connected to an adapter.
  OneWireMaster & oneWireMaster() { return oneWireMaster_; }

  /// Access the I2C master when connected to an adapter.
  I2CMaster & i2cMaster() { return i2cMaster_; }

private:
  class OneWireMasterImpl : public OneWireMasterDecorator {
  public:
    explicit OneWireMasterImpl(DS9481P_300 & parent)
        : OneWireMasterDecorator(parent.ds2480b), parent(&parent) {}

    MaximInterface_EXPORT virtual error_code reset();
    
    MaximInterface_EXPORT virtual error_code
    touchBitSetLevel(bool & sendRecvBit, Level afterLevel);
    
    MaximInterface_EXPORT virtual error_code
    writeByteSetLevel(uint_least8_t sendByte, Level afterLevel);
    
    MaximInterface_EXPORT virtual error_code
    readByteSetLevel(uint_least8_t & recvByte, Level afterLevel);
    
    MaximInterface_EXPORT virtual error_code
    writeBlock(span<const uint_least8_t> sendBuf);
    
    MaximInterface_EXPORT virtual error_code
    readBlock(span<uint_least8_t> recvBuf);
    
    MaximInterface_EXPORT virtual error_code setSpeed(Speed newSpeed);
    
    MaximInterface_EXPORT virtual error_code setLevel(Level newLevel);
    
    MaximInterface_EXPORT virtual error_code triplet(TripletData & data);

  private:
    DS9481P_300 * parent;
  };

  class I2CMasterImpl : public I2CMasterDecorator {
  public:
    explicit I2CMasterImpl(DS9481P_300 & parent)
        : I2CMasterDecorator(parent.ds9400), parent(&parent) {}

    MaximInterface_EXPORT virtual error_code start(uint_least8_t address);
    
    MaximInterface_EXPORT virtual error_code stop();
    
    MaximInterface_EXPORT virtual error_code writeByte(uint_least8_t data);
    
    MaximInterface_EXPORT virtual error_code
    writeBlock(span<const uint_least8_t> data);
    
    MaximInterface_EXPORT virtual error_code readByte(AckStatus status,
                                                      uint_least8_t & data);
    MaximInterface_EXPORT virtual error_code
    readBlock(AckStatus status, span<uint_least8_t> data);

  protected:
    MaximInterface_EXPORT virtual error_code
    writePacketImpl(uint_least8_t address, span<const uint_least8_t> data,
                    bool sendStop);
                    
    MaximInterface_EXPORT virtual error_code
    readPacketImpl(uint_least8_t address, span<uint_least8_t> data,
                   bool sendStop);

  private:
    DS9481P_300 * parent;
  };

  class DS2480BWithEscape : public DS2480B {
  public:
    DS2480BWithEscape(Sleep & sleep, Uart & uart) : DS2480B(sleep, uart) {}

    error_code escape();
  };

  class DS9400WithEscape : public DS9400 {
  public:
    explicit DS9400WithEscape(Uart & uart) : DS9400(uart) {}

    error_code escape();
  };

  enum Bus { OneWire, I2C };

  SerialPort * serialPort;
  Bus currentBus;
  DS2480BWithEscape ds2480b;
  OneWireMasterImpl oneWireMaster_;
  DS9400WithEscape ds9400;
  I2CMasterImpl i2cMaster_;

  error_code selectOneWire();
  error_code selectBus(Bus newBus);

  friend class OneWireMasterImpl;
  friend class I2CMasterImpl;
};

} // namespace MaximInterface

#endif
