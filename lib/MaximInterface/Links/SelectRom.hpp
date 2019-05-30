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

#ifndef MaximInterface_SelectRom
#define MaximInterface_SelectRom

#include <MaximInterface/Utilities/Export.h>
#include <MaximInterface/Utilities/Function.hpp>
#include <MaximInterface/Utilities/RomId.hpp>
#include <MaximInterface/Utilities/system_error.hpp>
#include "RomCommands.hpp"

namespace MaximInterface {

class OneWireMaster;

/// Selects a 1-Wire device on the bus for communication.
typedef Function<error_code(OneWireMaster &)> SelectRom;

/// Selector for a multidrop 1-Wire bus.
class SelectMatchRom {
public:
  typedef SelectRom::argument_type argument_type;
  typedef SelectRom::result_type result_type;

  explicit SelectMatchRom(RomId::const_span romId) { setRomId(romId); }

  RomId::const_span romId() const { return romId_; }
  
  void setRomId(RomId::const_span romId) { copy(romId, make_span(romId_)); }

  MaximInterface_EXPORT error_code operator()(OneWireMaster & master) const;

private:
  RomId::array romId_;
};

/// @brief
/// Selector for a multidrop 1-Wire bus where slaves support the Resume ROM
/// command.
class SelectMatchRomWithResume {
public:
  typedef SelectRom::argument_type argument_type;
  typedef SelectRom::result_type result_type;

  struct SharedData {
    SharedData() : lastRomId() {}
    RomId::array lastRomId;
  };

  SelectMatchRomWithResume(SharedData & data, RomId::const_span romId)
      : data(&data) {
    setRomId(romId);
  }

  void setSharedData(SharedData & data) { this->data = &data; }
  
  RomId::const_span romId() const { return romId_; }
  
  void setRomId(RomId::const_span romId) { copy(romId, make_span(romId_)); }

  MaximInterface_EXPORT error_code operator()(OneWireMaster & master) const;

private:
  SharedData * data;
  RomId::array romId_;
};

} // namespace MaximInterface

#endif
