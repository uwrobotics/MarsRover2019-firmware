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

#include "RomCommands.hpp"

namespace MaximInterface {

enum RomCmd {
  ReadRomCmd = 0x33,
  MatchRomCmd = 0x55,
  SearchRomCmd = 0xF0,
  SkipRomCmd = 0xCC,
  ResumeRomCmd = 0xA5,
  OverdriveSkipRomCmd = 0x3C,
  OverdriveMatchRomCmd = 0x69
};

void skipCurrentFamily(SearchRomState & searchState) {
  // Set the last discrepancy to last family discrepancy.
  searchState.lastDiscrepancy = searchState.lastFamilyDiscrepancy;
  // Clear the last family discrepancy.
  searchState.lastFamilyDiscrepancy = 0;
  // Check for end of list.
  if (searchState.lastDiscrepancy == 0) {
    searchState.lastDevice = true;
  }
}

error_code verifyRom(OneWireMaster & master, RomId::const_span romId) {
  SearchRomState searchState(romId);
  error_code result = searchRom(master, searchState);
  if (result) {
    return result;
  }
  // Check if same device found.
  if (romId != make_span(searchState.romId)) {
    result = make_error_code(OneWireMaster::NoSlaveError);
  }
  return result;
}

error_code readRom(OneWireMaster & master, RomId::span romId) {
  error_code result = master.reset();
  if (result) {
    return result;
  }
  result = master.writeByte(ReadRomCmd);
  if (result) {
    return result;
  }
  result = master.readBlock(romId);
  if (result) {
    return result;
  }
  if (!valid(romId)) {
    result = make_error_code(OneWireMaster::NoSlaveError);
  }
  return result;
}

error_code skipRom(OneWireMaster & master) {
  error_code result = master.reset();
  if (result) {
    return result;
  }
  result = master.writeByte(SkipRomCmd);
  return result;
}

error_code matchRom(OneWireMaster & master, RomId::const_span romId) {
  error_code result = master.reset();
  if (result) {
    return result;
  }
  result = master.writeByte(MatchRomCmd);
  if (result) {
    return result;
  }
  result = master.writeBlock(romId);
  return result;
}

error_code overdriveSkipRom(OneWireMaster & master) {
  error_code result = master.reset();
  if (result) {
    return result;
  }
  result = master.writeByte(OverdriveSkipRomCmd);
  if (result) {
    return result;
  }
  result = master.setSpeed(OneWireMaster::OverdriveSpeed);
  return result;
}

error_code overdriveMatchRom(OneWireMaster & master, RomId::const_span romId) {
  error_code result = master.reset();
  if (result) {
    return result;
  }
  result = master.writeByte(OverdriveMatchRomCmd);
  if (result) {
    return result;
  }
  result = master.setSpeed(OneWireMaster::OverdriveSpeed);
  if (result) {
    return result;
  }
  result = master.writeBlock(romId);
  return result;
}

error_code resumeRom(OneWireMaster & master) {
  error_code result = master.reset();
  if (result) {
    return result;
  }
  result = master.writeByte(ResumeRomCmd);
  return result;
}

error_code searchRom(OneWireMaster & master, SearchRomState & searchState) {
  if (searchState.lastDevice) {
    searchState = SearchRomState();
  }

  error_code result = master.reset();
  if (result) {
    return result;
  }
  result = master.writeByte(SearchRomCmd);
  if (result) {
    return result;
  }

  SearchRomState newSearchState;
  newSearchState.lastFamilyDiscrepancy = searchState.lastFamilyDiscrepancy;
  for (int idBitNumber = 1; idBitNumber <= 64; ++idBitNumber) {
    const int idByteNumber = (idBitNumber - 1) / 8;
    const unsigned int idBitMask = 1 << ((idBitNumber - 1) % 8);

    OneWireMaster::TripletData tripletData;
    if (idBitNumber == searchState.lastDiscrepancy) {
      tripletData.writeBit = 1;
    } else if (idBitNumber > searchState.lastDiscrepancy) {
      tripletData.writeBit = 0;
    } else { // idBitNumber < searchState.lastDiscrepancy
      tripletData.writeBit =
          (searchState.romId[idByteNumber] & idBitMask) == idBitMask;
    }

    result = master.triplet(tripletData);
    if (result) {
      return result;
    }
    if (tripletData.readBit && tripletData.readBitComplement) {
      return make_error_code(OneWireMaster::NoSlaveError);
    }

    if (tripletData.writeBit) {
      newSearchState.romId[idByteNumber] |= idBitMask;
    } else if (!tripletData.readBit && !tripletData.readBitComplement) {
      newSearchState.lastDiscrepancy = idBitNumber;
      if (idBitNumber <= 8) {
        newSearchState.lastFamilyDiscrepancy = idBitNumber;
      }
    }
  }

  if (valid(newSearchState.romId)) {
    if (newSearchState.lastDiscrepancy == 0) {
      newSearchState.lastDevice = true;
    }
    searchState = newSearchState;
  } else {
    result = make_error_code(OneWireMaster::NoSlaveError);
  }
  return result;
}

} // namespace MaximInterface
