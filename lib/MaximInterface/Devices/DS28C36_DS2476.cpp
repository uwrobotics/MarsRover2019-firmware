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
#include <MaximInterface/Links/I2CMaster.hpp>
#include <MaximInterface/Utilities/Error.hpp>
#include <MaximInterface/Utilities/RomId.hpp>
#include "DS28C36_DS2476.hpp"

namespace MaximInterface {

using namespace Ecc256;
using std::copy;

static error_code convertResultByte(uint_least8_t resultByte) {
  error_code errorCode;
  if (resultByte != 0xAA) {
    errorCode.assign((resultByte == 0x00)
                         ? static_cast<int>(DS28C36::AuthenticationError)
                         : resultByte,
                     DS28C36::errorCategory());
  }
  return errorCode;
}

const int DS28C36::publicKeyAxPage;
const int DS28C36::publicKeyAyPage;
const int DS28C36::publicKeyBxPage;
const int DS28C36::publicKeyByPage;
const int DS28C36::publicKeyCxPage;
const int DS28C36::publicKeyCyPage;
const int DS28C36::privateKeyAPage;
const int DS28C36::privateKeyBPage;
const int DS28C36::privateKeyCPage;
const int DS28C36::secretAPage;
const int DS28C36::secretBPage;
const int DS28C36::decrementCounterPage;
const int DS28C36::romOptionsPage;
const int DS28C36::gpioControlPage;
const int DS28C36::publicKeySxPage;
const int DS28C36::publicKeySyPage;
const int DS28C36::memoryPages;

error_code DS28C36::writeMemory(int pageNum, Page::const_span page) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[1 + Page::size];
  buffer[0] = pageNum;
  copy(page.begin(), page.end(), buffer + 1);
  error_code result = writeCommand(0x96, buffer);
  if (!result) {
    sleep(writeMemoryTimeMs);
    result = readFixedLengthResponse(make_span(buffer, 1));
    if (!result) {
      result = convertResultByte(buffer[0]);
    }
  }
  return result;
}

error_code DS28C36::readMemory(int pageNum, Page::span page) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  const uint_least8_t parameter = pageNum;
  error_code result = writeCommand(0x69, make_span(&parameter, 1));
  if (!result) {
    sleep(readMemoryTimeMs);
    array<uint_least8_t, 1 + Page::size> response;
    result = readFixedLengthResponse(response);
    if (!result) {
      result = convertResultByte(response[0]);
      copy(response.begin() + 1, response.end(), page.begin());
    }
  }
  return result;
}

error_code DS28C36::writeBuffer(span<const uint_least8_t> data) {
  return writeCommand(0x87, data);
}

error_code DS28C36::readBuffer(std::vector<uint_least8_t> & data) {
  error_code result = writeCommand(0x5A);
  if (!result) {
    data.resize(80);
    span<uint_least8_t> dataSpan(data);
    result = readVariableLengthResponse(dataSpan);
    if (result) {
      data.clear();
    } else {
      data.resize(dataSpan.size());
    }
  }
  return result;
}

error_code DS28C36::readPageProtection(int pageNum,
                                       PageProtection & protection) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer = pageNum;
  error_code result = writeCommand(0xAA, make_span(&buffer, 1));
  if (!result) {
    sleep(readMemoryTimeMs);
    result = readFixedLengthResponse(make_span(&buffer, 1));
    if (!result) {
      protection = buffer;
    }
  }
  return result;
}

error_code DS28C36::setPageProtection(int pageNum,
                                      const PageProtection & protection) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[] = {static_cast<uint_least8_t>(pageNum),
                            static_cast<uint_least8_t>(protection.to_ulong())};
  error_code result = writeCommand(0xC3, buffer);
  if (!result) {
    sleep(writeMemoryTimeMs);
    result = readFixedLengthResponse(make_span(buffer, 1));
    if (!result) {
      result = convertResultByte(buffer[0]);
    }
  }
  return result;
}

error_code DS28C36::decrementCounter() {
  error_code result = writeCommand(0xC9);
  if (!result) {
    sleep(writeMemoryTimeMs);
    uint_least8_t response;
    result = readFixedLengthResponse(make_span(&response, 1));
    if (!result) {
      result = convertResultByte(response);
    }
  }
  return result;
}

error_code DS28C36::readRng(span<uint_least8_t> data) {
  if ((data.size() < 1) || (data.size() > 64)) {
    return make_error_code(InvalidParameterError);
  }

  data[0] = static_cast<uint_least8_t>(data.size() - 1);
  error_code result = writeCommand(0xD2, data.first(1));
  if (!result) {
    sleep(sha256ComputationTimeMs);
    result = readFixedLengthResponse(data);
  }
  return result;
}

error_code DS28C36::encryptedReadMemory(int pageNum, SecretNum secretNum,
                                        EncryptionChallenge::span challenge,
                                        Page::span data) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  const uint_least8_t parameter = (secretNum << 6) | pageNum;
  error_code result = writeCommand(0x4B, make_span(&parameter, 1));
  if (!result) {
    sleep(readMemoryTimeMs + sha256ComputationTimeMs);
    uint_least8_t response[1 + EncryptionChallenge::size + Page::size];
    result = readFixedLengthResponse(response);
    if (!result) {
      result = convertResultByte(response[0]);
      const uint_least8_t * begin = response + 1;
      const uint_least8_t * end = begin + challenge.size();
      copy(begin, end, challenge.begin());
      begin = end;
      end = begin + data.size();
      copy(begin, end, data.begin());
    }
  }
  return result;
}

error_code DS28C36::computeAndReadPageAuthentication(int pageNum,
                                                     AuthType authType) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  const uint_least8_t parameter = (authType << 5) | pageNum;
  return writeCommand(0xA5, make_span(&parameter, 1));
}

error_code
DS28C36::computeAndReadPageAuthentication(int pageNum, KeyNum keyNum,
                                          Signature::span signature) {
  AuthType authType;
  switch (keyNum) {
  case KeyNumA:
    authType = EcdsaWithKeyA;
    break;
  case KeyNumB:
    authType = EcdsaWithKeyB;
    break;
  case KeyNumC:
    authType = EcdsaWithKeyC;
    break;
  default:
    return make_error_code(InvalidParameterError);
  }
  error_code result = computeAndReadPageAuthentication(pageNum, authType);
  if (!result) {
    sleep(readMemoryTimeMs + generateEcdsaSignatureTimeMs);
    uint_least8_t response[1 + 2 * Scalar::size];
    result = readFixedLengthResponse(response);
    if (!result) {
      result = convertResultByte(response[0]);
      const uint_least8_t * begin = response + 1;
      const uint_least8_t * end = begin + signature.s.size();
      copy(begin, end, signature.s.begin());
      begin = end;
      end = begin + signature.r.size();
      copy(begin, end, signature.r.begin());
    }
  }
  return result;
}

error_code DS28C36::computeAndReadPageAuthentication(int pageNum,
                                                     SecretNum secretNum,
                                                     Sha256::Hash::span hmac) {
  AuthType authType;
  switch (secretNum) {
  case SecretNumA:
    authType = HmacWithSecretA;
    break;
  case SecretNumB:
    authType = HmacWithSecretB;
    break;
  case SecretNumS:
    authType = HmacWithSecretS;
    break;
  default:
    return make_error_code(InvalidParameterError);
  }
  error_code result = computeAndReadPageAuthentication(pageNum, authType);
  if (!result) {
    sleep(readMemoryTimeMs + sha256ComputationTimeMs);
    array<uint_least8_t, 1 + Sha256::Hash::size> response;
    result = readFixedLengthResponse(response);
    if (!result) {
      result = convertResultByte(response[0]);
      copy(response.begin() + 1, response.end(), hmac.begin());
    }
  }
  return result;
}

error_code DS28C36::authenticatedSha2WriteMemory(int pageNum,
                                                 SecretNum secretNum,
                                                 Page::const_span page) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[1 + Page::size];
  buffer[0] = (secretNum << 6) | pageNum;
  copy(page.begin(), page.end(), buffer + 1);
  error_code result = writeCommand(0x99, buffer);
  if (!result) {
    sleep(writeMemoryTimeMs + (2 * sha256ComputationTimeMs));
    result = readFixedLengthResponse(make_span(buffer, 1));
    if (!result) {
      result = convertResultByte(buffer[0]);
    }
  }
  return result;
}

error_code DS28C36::computeAndLockSha2Secret(int pageNum, SecretNum msecretNum,
                                             SecretNum dsecretNum,
                                             bool writeProtectEnable) {
  // User pages only
  if (pageNum < 0 || pageNum > 15) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[] = {
      static_cast<uint_least8_t>((dsecretNum << 6) | (msecretNum << 4) |
                                 pageNum),
      static_cast<uint_least8_t>(writeProtectEnable ? 0x80 : 0x00)};
  error_code result = writeCommand(0x3C, buffer);
  if (!result) {
    sleep(sha256ComputationTimeMs +
          ((writeProtectEnable ? 2 : 1) * writeMemoryTimeMs));
    result = readFixedLengthResponse(make_span(buffer, 1));
    if (!result) {
      result = convertResultByte(buffer[0]);
    }
  }
  return result;
}

error_code DS28C36::generateEcc256KeyPair(KeyNum keyNum,
                                          bool writeProtectEnable) {
  if (keyNum == KeyNumS) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer = keyNum;
  if (writeProtectEnable) {
    buffer |= 0x80;
  }
  error_code result = writeCommand(0xCB, make_span(&buffer, 1));
  if (!result) {
    sleep(generateEccKeyPairTimeMs);
    result = readFixedLengthResponse(make_span(&buffer, 1));
    if (!result) {
      result = convertResultByte(buffer);
    }
  }
  return result;
}

error_code DS28C36::computeMultiblockHash(bool firstBlock, bool lastBlock,
                                          span<const uint_least8_t> data) {
  const span<const uint_least8_t>::index_type maxDataSize = 64;

  if (data.size() < 1 || data.size() > maxDataSize) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[1 + maxDataSize];
  buffer[0] = 0;
  if (firstBlock) {
    buffer[0] |= 0x40;
  }
  if (lastBlock) {
    buffer[0] |= 0x80;
  }
  copy(data.begin(), data.end(), buffer + 1);
  error_code result = writeCommand(0x33, make_span(buffer, data.size() + 1));
  if (!result) {
    sleep(sha256ComputationTimeMs);
    result = readFixedLengthResponse(make_span(buffer, 1));
    if (!result) {
      result = convertResultByte(buffer[0]);
    }
  }
  return result;
}

error_code DS28C36::verifyEcdsaSignature(KeyNum keyNum, HashType hashType,
                                         Signature::const_span signature,
                                         PioState pioa, PioState piob) {
  uint_least8_t buffer[1 + 2 * Scalar::size];
  buffer[0] = keyNum | (hashType << 2);
  if (pioa != Unchanged) {
    buffer[0] |= 0x20;
  }
  if (pioa == Conducting) {
    buffer[0] |= 0x10;
  }
  if (piob != Unchanged) {
    buffer[0] |= 0x80;
  }
  if (piob == Conducting) {
    buffer[0] |= 0x40;
  }
  uint_least8_t * bufferIt =
      copy(signature.r.begin(), signature.r.end(), buffer + 1);
  copy(signature.s.begin(), signature.s.end(), bufferIt);
  error_code result = writeCommand(0x59, buffer);
  if (!result) {
    sleep(verifyEsdsaSignatureOrComputeEcdhTimeMs +
          ((hashType == DataInBuffer) ? sha256ComputationTimeMs : 0));
    result = readFixedLengthResponse(make_span(buffer, 1));
    if (!result) {
      result = convertResultByte(buffer[0]);
    }
  }
  return result;
}

error_code
DS28C36::authenticateEcdsaPublicKey(bool authWrites, bool ecdh, KeyNum keyNum,
                                    int csOffset,
                                    Signature::const_span signature) {
  if (((keyNum != KeyNumA) && (keyNum != KeyNumB)) || (csOffset < 0) ||
      (csOffset > 31)) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[1 + 2 * Scalar::size];
  buffer[0] = (csOffset << 3) | (keyNum << 2);
  if (ecdh) {
    buffer[0] |= 0x02;
  }
  if (authWrites) {
    buffer[0] |= 0x01;
  }
  uint_least8_t * bufferIt =
      copy(signature.r.begin(), signature.r.end(), buffer + 1);
  copy(signature.s.begin(), signature.s.end(), bufferIt);
  error_code result = writeCommand(0xA8, buffer);
  if (!result) {
    sleep((ecdh ? 2 : 1) * verifyEsdsaSignatureOrComputeEcdhTimeMs);
    result = readFixedLengthResponse(make_span(buffer, 1));
    if (!result) {
      result = convertResultByte(buffer[0]);
    }
  }
  return result;
}

error_code DS28C36::authenticatedEcdsaWriteMemory(int pageNum,
                                                  Page::const_span page) {
  if (pageNum < 0 || pageNum >= memoryPages) {
    return make_error_code(InvalidParameterError);
  }

  uint_least8_t buffer[1 + Page::size];
  buffer[0] = pageNum;
  copy(page.begin(), page.end(), buffer + 1);
  error_code result = writeCommand(0x89, buffer);
  if (!result) {
    sleep(verifyEsdsaSignatureOrComputeEcdhTimeMs + writeMemoryTimeMs +
          sha256ComputationTimeMs);
    result = readFixedLengthResponse(make_span(buffer, 1));
    if (!result) {
      result = convertResultByte(buffer[0]);
    }
  }
  return result;
}

error_code DS28C36::writeCommand(uint_least8_t command,
                                 span<const uint_least8_t> parameters) {
  error_code result = master->start(address_);
  if (result) {
    master->stop();
    return result;
  }
  result = master->writeByte(command);
  if (result) {
    master->stop();
    return result;
  }
  if (!parameters.empty()) {
    result = master->writeByte(static_cast<uint_least8_t>(parameters.size()));
    if (result) {
      master->stop();
      return result;
    }
    result = master->writeBlock(parameters);
    if (result) {
      master->stop();
      return result;
    }
  }
  result = master->stop();
  return result;
}

error_code DS28C36::readVariableLengthResponse(span<uint_least8_t> & response) {
  error_code result = master->start(address_ | 1);
  if (result) {
    master->stop();
    return result;
  }
  uint_least8_t length;
  result = master->readByte(I2CMaster::Ack, length);
  if (result) {
    master->stop();
    return result;
  }
  if (length > response.size()) {
    master->stop();
    return make_error_code(InvalidResponseError);
  }
  response = response.first(length);
  if (!response.empty()) {
    result = master->readBlock(I2CMaster::Nack, response);
    if (result) {
      master->stop();
      return result;
    }
  }
  result = master->stop();
  return result;
}

error_code DS28C36::readFixedLengthResponse(span<uint_least8_t> response) {
  const span<uint_least8_t>::index_type requestedResponseSize = response.size();
  error_code result = readVariableLengthResponse(response);
  if (!result && response.size() != requestedResponseSize) {
    result = make_error_code(InvalidResponseError);
  }
  return result;
}

const error_category & DS28C36::errorCategory() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "DS28C36"; }

    virtual std::string message(int condition) const {
      switch (condition) {
      case ProtectionError:
        return "Protection Error";

      case InvalidParameterError:
        return "Invalid Parameter Error";

      case InvalidSequenceError:
        return "Invalid Sequence Error";

      case InvalidEcdsaInputOrResultError:
        return "Invalid ECDSA Input or Result Error";

      case AuthenticationError:
        return "Authentication Error";

      case InvalidResponseError:
        return "Invalid Response Error";

      default:
        return defaultErrorMessage(condition);
      }
    }
  } instance;
  return instance;
}

error_code DS2476::generateEcdsaSignature(KeyNum keyNum,
                                          Signature::span signature) {
  if (keyNum == KeyNumS) {
    return make_error_code(InvalidParameterError);
  }

  const uint_least8_t parameter = keyNum;
  error_code result = writeCommand(0x1E, make_span(&parameter, 1));
  if (!result) {
    sleep(generateEcdsaSignatureTimeMs);
    uint_least8_t response[1 + 2 * Scalar::size];
    result = readFixedLengthResponse(response);
    if (!result) {
      result = convertResultByte(response[0]);
      const uint_least8_t * begin = response + 1;
      const uint_least8_t * end = begin + signature.s.size();
      copy(begin, end, signature.s.begin());
      begin = end;
      end = begin + signature.r.size();
      copy(begin, end, signature.r.begin());
    }
  }
  return result;
}

error_code DS2476::computeSha2UniqueSecret(SecretNum msecretNum) {
  uint_least8_t buffer = msecretNum << 4;
  error_code result = writeCommand(0x55, make_span(&buffer, 1));
  if (!result) {
    sleep(sha256ComputationTimeMs);
    result = readFixedLengthResponse(make_span(&buffer, 1));
    if (!result) {
      convertResultByte(buffer);
    }
  }
  return result;
}

error_code DS2476::computeSha2Hmac(Sha256::Hash::span hmac) {
  error_code result = writeCommand(0x2D);
  if (!result) {
    sleep(sha256ComputationTimeMs);
    array<uint_least8_t, 1 + Sha256::Hash::size> response;
    result = readFixedLengthResponse(response);
    if (!result) {
      result = convertResultByte(response[0]);
      copy(response.begin() + 1, response.end(), hmac.begin());
    }
  }
  return result;
}

error_code computeMultiblockHash(DS28C36 & ds28c36,
                                 span<const uint_least8_t> data) {
  error_code result;
  span<const uint_least8_t>::index_type dataIdx = 0;
  while (dataIdx < data.size() && !result) {
    const span<const uint_least8_t>::index_type remainingSize =
        data.size() - dataIdx;
    const span<const uint_least8_t>::index_type chunkSize =
        std::min<span<const uint_least8_t>::index_type>(remainingSize, 64);
    result =
        ds28c36.computeMultiblockHash(dataIdx == 0, remainingSize == chunkSize,
                                      data.subspan(dataIdx, chunkSize));
    dataIdx += chunkSize;
  }
  return result;
}

error_code verifyEcdsaSignature(DS28C36 & ds28c36, DS28C36::KeyNum publicKey,
                                span<const uint_least8_t> data,
                                Signature::const_span signature,
                                DS28C36::PioState pioa,
                                DS28C36::PioState piob) {
  error_code result = computeMultiblockHash(ds28c36, data);
  if (!result) {
    result = ds28c36.verifyEcdsaSignature(publicKey, DS28C36::THASH, signature,
                                          pioa, piob);
  }
  return result;
}

error_code verifyEcdsaSignature(DS28C36 & ds28c36,
                                PublicKey::const_span publicKey,
                                span<const uint_least8_t> data,
                                Signature::const_span signature,
                                DS28C36::PioState pioa,
                                DS28C36::PioState piob) {
  error_code result =
      ds28c36.writeMemory(DS28C36::publicKeySxPage, publicKey.x);
  if (!result) {
    result = ds28c36.writeMemory(DS28C36::publicKeySyPage, publicKey.y);
  }
  if (!result) {
    result = verifyEcdsaSignature(ds28c36, DS28C36::KeyNumS, data, signature,
                                  pioa, piob);
  }
  return result;
}

error_code readRomIdAndManId(DS28C36 & ds28c36, RomId::span romId,
                             ManId::span manId) {
  DS28C36::Page::array page;
  error_code result = ds28c36.readMemory(DS28C36::romOptionsPage, page);
  if (!result) {
    const DS28C36::RomOptions romOptions(page);
    copy(romOptions.romId(), romId);
    copy(romOptions.manId(), manId);
  }
  return result;
}

error_code enableCoprocessor(DS2476 & ds2476) {
  DS2476::Page::array page;
  error_code result = ds2476.readMemory(DS2476::gpioControlPage, page);
  if (!result) {
    DS2476::GpioControl gpioControl(page);
    if (!gpioControl.pioaConducting()) {
      gpioControl.setPioaConducting(true);
      result = ds2476.writeMemory(DS2476::gpioControlPage, page);
    }
  }
  return result;
}

error_code enableRomId(DS2476 & ds2476) {
  DS2476::Page::array page;
  error_code result = ds2476.readMemory(DS2476::romOptionsPage, page);
  if (!result) {
    DS2476::RomOptions romOptions(page);
    if (!romOptions.romBlockDisable()) {
      romOptions.setRomBlockDisable(true);
      result = ds2476.writeMemory(DS2476::romOptionsPage, page);
    }
  }
  return result;
}

static void setAnonymous(RomId::span romId) {
  std::fill(romId.begin(), romId.end(), 0xFF);
}

DS28C36::PageAuthenticationData &
DS28C36::PageAuthenticationData::setAnonymousRomId() {
  setAnonymous(romId());
  return *this;
}

DS28C36::EncryptionHmacData & DS28C36::EncryptionHmacData::setAnonymousRomId() {
  setAnonymous(romId());
  return *this;
}

} // namespace MaximInterface
