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

#ifndef MaximInterface_Ecc256
#define MaximInterface_Ecc256

#include <stdint.h>
#include "array_span.hpp"
#include "Export.h"
#include "ManId.hpp"
#include "RomId.hpp"

namespace MaximInterface {
namespace Ecc256 {

typedef array_span<uint_least8_t, 32> Scalar;

struct Point {
  struct const_span {
    Scalar::const_span x;
    Scalar::const_span y;
  };

  struct span {
    Scalar::span x;
    Scalar::span y;

    operator const_span() const {
      const const_span sp = {x, y};
      return sp;
    }
  };

  struct array {
    Scalar::array x;
    Scalar::array y;

    operator span() {
      const span sp = {x, y};
      return sp;
    }
    
    operator const_span() const {
      const const_span sp = {x, y};
      return sp;
    }
  };
  
private:
  Point(); // deleted
};

MaximInterface_EXPORT void copy(Point::const_span src, Point::span dst);

typedef Scalar PrivateKey;
typedef Point PublicKey;

struct KeyPair {
  struct const_span {
    PrivateKey::const_span privateKey;
    PublicKey::const_span publicKey;
  };

  struct span {
    PrivateKey::span privateKey;
    PublicKey::span publicKey;

    operator const_span() const {
      const const_span sp = {privateKey, publicKey};
      return sp;
    }
  };

  struct array {
    PrivateKey::array privateKey;
    PublicKey::array publicKey;

    operator span() {
      const span sp = {privateKey, publicKey};
      return sp;
    }
    
    operator const_span() const {
      const const_span sp = {privateKey, publicKey};
      return sp;
    }
  };
  
private:
  KeyPair(); // deleted
};

MaximInterface_EXPORT void copy(KeyPair::const_span src, KeyPair::span dst);

struct Signature {
  struct const_span {
    Scalar::const_span r;
    Scalar::const_span s;
  };

  struct span {
    Scalar::span r;
    Scalar::span s;

    operator const_span() const {
      const const_span sp = {r, s};
      return sp;
    }
  };

  struct array {
    Scalar::array r;
    Scalar::array s;

    operator span() {
      const span sp = {r, s};
      return sp;
    }
    
    operator const_span() const {
      const const_span sp = {r, s};
      return sp;
    }
  };
  
private:
  Signature(); // deleted
};

MaximInterface_EXPORT void copy(Signature::const_span src, Signature::span dst);

/// Data used to create a device key certificate for ECC-256 authenticators.
class CertificateData {
public:
  typedef array_span<uint_least8_t, 2 * Scalar::size + RomId::size + ManId::size>
      Result;

  CertificateData() : result_() {}

  /// Formatted data result.
  Result::const_span result() const { return result_; }

  /// @name Public Key
  /// @brief Public key of the device.
  /// @{
  
  /// Get mutable Public Key.
  MaximInterface_EXPORT PublicKey::span publicKey();
  
  /// Get immutable Public Key.
  PublicKey::const_span publicKey() const {
    return const_cast<CertificateData &>(*this).publicKey();
  }
  
  /// Set Public Key.
  CertificateData & setPublicKey(PublicKey::const_span publicKey) {
    copy(publicKey, this->publicKey());
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
    return const_cast<CertificateData &>(*this).romId();
  }
  
  /// Set ROM ID.
  CertificateData & setRomId(RomId::const_span romId) {
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
    return const_cast<CertificateData &>(*this).manId();
  }
  
  /// Set MAN ID.
  CertificateData & setManId(ManId::const_span manId) {
    copy(manId, this->manId());
    return *this;
  }
  
  /// @}

private:
  typedef Result::span::index_type index;

  static const index publicKeyIdx = 0;
  static const index romIdIdx = publicKeyIdx + 2 * Scalar::size;
  static const index manIdIdx = romIdIdx + RomId::size;

  Result::array result_;
};

} // namespace Ecc256
} // namespace MaximInterface

#endif
