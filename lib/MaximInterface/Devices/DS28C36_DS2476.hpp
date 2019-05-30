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

#ifndef MaximInterface_DS28C36_DS2476
#define MaximInterface_DS28C36_DS2476

#include <stdint.h>
#include <vector>
#include <MaximInterface/Links/I2CMaster.hpp>
#include <MaximInterface/Links/Sleep.hpp>
#include <MaximInterface/Utilities/array_span.hpp>
#include <MaximInterface/Utilities/Ecc256.hpp>
#include <MaximInterface/Utilities/Export.h>
#include <MaximInterface/Utilities/RomId.hpp>
#include <MaximInterface/Utilities/ManId.hpp>
#include <MaximInterface/Utilities/Sha256.hpp>
#include <MaximInterface/Utilities/system_error.hpp>
#include <MaximInterface/Utilities/FlagSet.hpp>

namespace MaximInterface {

/// Interface to the DS28C36 authenticator.
class DS28C36 {
public:
  /// Device command results.
  enum ErrorValue {
    ProtectionError = 0x55,
    InvalidParameterError = 0x77,
    InvalidSequenceError = 0x33,
    InvalidEcdsaInputOrResultError = 0x22,
    AuthenticationError = 0x100,
    InvalidResponseError = 0x101 ///< Response does not match expected format.
  };

  /// @name Device memory pages
  /// @{
  
  static const int publicKeyAxPage = 16;
  static const int publicKeyAyPage = 17;
  static const int publicKeyBxPage = 18;
  static const int publicKeyByPage = 19;
  static const int publicKeyCxPage = 20;
  static const int publicKeyCyPage = 21;
  static const int privateKeyAPage = 22;
  static const int privateKeyBPage = 23;
  static const int privateKeyCPage = 24;
  static const int secretAPage = 25;
  static const int secretBPage = 26;
  static const int decrementCounterPage = 27;
  static const int romOptionsPage = 28;
  static const int gpioControlPage = 29;
  static const int publicKeySxPage = 30;
  static const int publicKeySyPage = 31;
  
  /// @}

  /// Number of memory pages on the device.
  static const int memoryPages = 32;

  /// Available keys for ECDSA operations.
  enum KeyNum { KeyNumA = 0, KeyNumB = 1, KeyNumC = 2, KeyNumS = 3 };

  /// Available secrets for HMAC operations.
  enum SecretNum { SecretNumA = 0, SecretNumB = 1, SecretNumS = 2 };

  /// Data hash type when verifying an ECDSA signature.
  enum HashType {
    HashInBuffer = 0, ///< Hash is loaded in the buffer.
    DataInBuffer = 1, ///< Compute hash from data loaded in the buffer.
    THASH = 2         ///< Use THASH from Compute Multiblock Hash command.
  };

  /// Available PIO states when verifying an ECDSA signature.
  enum PioState { Unchanged, Conducting, HighImpedance };

  /// Holds a device memory page.
  typedef array_span<uint_least8_t, 32> Page;

  // Format page authentication input data.
  class PageAuthenticationData;

  // Format authenticated write input data.
  class WriteAuthenticationData;

  // Format compute secret input data.
  class ComputeSecretData;

  // Format encryption or decryption HMAC input data.
  class EncryptionHmacData;

  // Access fields in the ROM Options page.
  class RomOptions;

  // Access fields in the GPIO Control page.
  class GpioControl;

  /// Page protection types.
  enum PageProtectionType {
    RP = 0x01,   ///< Read protection.
    WP = 0x02,   ///< Write protection.
    EM = 0x04,   ///< EPROM emulation mode.
    APH = 0x08,  ///< Authentication write protection HMAC.
    EPH = 0x10,  ///< Encryption and authenticated write protection HMAC.
    AUTH = 0x20, ///< Public Key C is set to authority public key.
    ECH = 0x40,  ///< Encrypted read and write using shared key from ECDH.
    ECW = 0x80   ///< Authentication write protection ECDSA.
  };
  typedef FlagSet<PageProtectionType, 8> PageProtection;

  /// Challenge for an encrypted device memory page.
  typedef array_span<uint_least8_t, 8> EncryptionChallenge;

  DS28C36(Sleep & sleep, I2CMaster & master, uint_least8_t address = 0x36)
      : sleep_(&sleep), master(&master), address_(address & 0xFE) {}

  void setSleep(Sleep & sleep) { sleep_ = &sleep; }

  void setMaster(I2CMaster & master) { this->master = &master; }

  uint_least8_t address() const { return address_; }

  void setAddress(uint_least8_t address) { address_ = address & 0xFE; }

  /// @brief Write memory with no protection.
  /// @param pageNum Number of page to write.
  /// @param page Data to write.
  MaximInterface_EXPORT error_code writeMemory(int pageNum,
                                               Page::const_span page);

  /// @brief Read memory with no protection.
  /// @param pageNum Number of page to read.
  /// @param[out] page Data that was read.
  MaximInterface_EXPORT error_code readMemory(int pageNum, Page::span page);

  /// @brief Write the temporary buffer.
  /// @param data Data to write.
  MaximInterface_EXPORT error_code writeBuffer(span<const uint_least8_t> data);

  /// @brief Read the temporary buffer.
  /// @param[out] data Data that was read.
  MaximInterface_EXPORT error_code
  readBuffer(std::vector<uint_least8_t> & data);

  /// @brief Read the protection settings of a page.
  /// @param pageNum Number of page to read.
  /// @param[out] protection Protection that was read.
  MaximInterface_EXPORT error_code
  readPageProtection(int pageNum, PageProtection & protection);

  /// @brief Set the protection settings of a page.
  /// @param pageNum Number of page to write.
  /// @param protection Protection to write.
  MaximInterface_EXPORT error_code
  setPageProtection(int pageNum, const PageProtection & protection);

  /// Decrement the decrement-only counter.
  MaximInterface_EXPORT error_code decrementCounter();

  /// @brief Read a block of random data from the RNG.
  /// @param[out] data Random data from RNG with length from 1 to 64.
  MaximInterface_EXPORT error_code readRng(span<uint_least8_t> data);

  /// @brief Read memory with encryption.
  /// @param pageNum Number of page to read from.
  /// @param secretNum Secret to use for encryption.
  /// @param[out] challenge Encryption challenge that was read.
  /// @param[out] data Encrypted page data that was read.
  MaximInterface_EXPORT error_code
  encryptedReadMemory(int pageNum, SecretNum secretNum,
                      EncryptionChallenge::span challenge, Page::span data);

  /// @brief Compute and read page authentication with ECDSA.
  /// @param pageNum Number of page to authenticate.
  /// @param keyNum
  /// Private key to use for authentication.
  /// Key S cannot be used with this command.
  /// @param[out] signature Computed page signature.
  MaximInterface_EXPORT error_code computeAndReadPageAuthentication(
      int pageNum, KeyNum keyNum, Ecc256::Signature::span signature);

  /// @brief Compute and read page authentication with HMAC.
  /// @param pageNum Number of page to authenticate.
  /// @param secretNum Secret to use for authentication.
  /// @param[out] hmac Computed page HMAC.
  MaximInterface_EXPORT error_code computeAndReadPageAuthentication(
      int pageNum, SecretNum secretNum, Sha256::Hash::span hmac);

  /// @brief Write with SHA2 authentication.
  /// @param pageNum Number of page to write.
  /// @param secretNum Secret to use for authentication.
  /// @param page Data to write.
  MaximInterface_EXPORT error_code authenticatedSha2WriteMemory(
      int pageNum, SecretNum secretNum, Page::const_span page);

  /// @brief Compute SHA2 secret and optionally lock.
  /// @param pageNum Number of page to use in computation.
  /// @param msecretNum Master secret to use in computation.
  /// @param dsecretNum Destination secret to receive the computation result.
  /// @param writeProtectEnable
  /// True to lock the destination secret against further writes.
  MaximInterface_EXPORT error_code
  computeAndLockSha2Secret(int pageNum, SecretNum msecretNum,
                           SecretNum dsecretNum, bool writeProtectEnable);

  /// @brief Generate a new ECDSA key pair.
  /// @param keyNum Key to generate. Key S cannot be used with this command.
  /// @param writeProtectEnable True to lock the key against further writes.
  MaximInterface_EXPORT error_code
  generateEcc256KeyPair(KeyNum keyNum, bool writeProtectEnable);

  /// @brief Compute a hash over multiple blocks.
  /// @param firstBlock True if this is the first block being hashed.
  /// @param lastBlock True if this is the last block being hashed.
  /// @param data
  /// Data block to hash. Should be 64 bytes unless this is the last block.
  MaximInterface_EXPORT error_code computeMultiblockHash(
      bool firstBlock, bool lastBlock, span<const uint_least8_t> data);

  /// @brief Verify ECDSA signature.
  /// @param keyNum Public key to use for verification.
  /// @param hashType Source of the data hash input.
  /// @param signature Signature to verify.
  /// @param pioa New state of PIOA if verification successful.
  /// @param piob New state of PIOB if verification successful.
  MaximInterface_EXPORT error_code verifyEcdsaSignature(
      KeyNum keyNum, HashType hashType, Ecc256::Signature::const_span signature,
      PioState pioa = Unchanged, PioState piob = Unchanged);

  /// @brief
  /// Authenticate a public key for authenticated writes or encrypted reads
  /// with ECDH.
  /// @param authWrites True to select authentication for writes.
  /// @param ecdh True to select ECDH key exchange.
  /// @param keyNum
  /// Private key to use for ECDH key exchange. Key A or B can be selected.
  /// @param csOffset Certificate customization field ending offset in buffer.
  /// @param signature Signature to use for authentication of public key S.
  MaximInterface_EXPORT error_code authenticateEcdsaPublicKey(
      bool authWrites, bool ecdh, KeyNum keyNum, int csOffset,
      Ecc256::Signature::const_span signature);

  /// @brief Write with ECDSA authentication.
  /// @param pageNum Number of page to write.
  /// @param page Data to write.
  MaximInterface_EXPORT error_code
  authenticatedEcdsaWriteMemory(int pageNum, Page::const_span page);

  MaximInterface_EXPORT static const error_category & errorCategory();

protected:
  // Timing constants.
  static const int generateEcdsaSignatureTimeMs = 50;
  static const int generateEccKeyPairTimeMs = 100;
  static const int verifyEsdsaSignatureOrComputeEcdhTimeMs = 150;
  static const int sha256ComputationTimeMs = 3;
  static const int readMemoryTimeMs = /*1*/ 2;
  static const int writeMemoryTimeMs = 15;

  error_code writeCommand(uint_least8_t command,
                          span<const uint_least8_t> parameters);

  error_code writeCommand(uint_least8_t command) {
    return writeCommand(command, span<const uint_least8_t>());
  }

  error_code readVariableLengthResponse(span<uint_least8_t> & response);

  error_code readFixedLengthResponse(span<uint_least8_t> response);

  void sleep(int ms) const { sleep_->invoke(ms); }

private:
  enum AuthType {
    HmacWithSecretA = 0,
    HmacWithSecretB = 1,
    HmacWithSecretS = 2,
    EcdsaWithKeyA = 3,
    EcdsaWithKeyB = 4,
    EcdsaWithKeyC = 5
  };

  const Sleep * sleep_;
  I2CMaster * master;
  uint_least8_t address_;

  error_code computeAndReadPageAuthentication(int pageNum, AuthType authType);
};

/// Interface to the DS2476 coprocessor.
class DS2476 : public DS28C36 {
public:
  DS2476(Sleep & sleep, I2CMaster & master, uint_least8_t address = 0x76)
      : DS28C36(sleep, master, address) {}

  /// @brief Generate ECDSA signature.
  /// @param keyNum
  /// Private key to use to create signature.
  /// Key S cannot be used with this command.
  /// @param[out] signature Computed signature.
  MaximInterface_EXPORT error_code
  generateEcdsaSignature(KeyNum keyNum, Ecc256::Signature::span signature);

  /// @brief Compute unique SHA2 secret.
  /// @param msecretNum Master secret to use in computation.
  MaximInterface_EXPORT error_code
  computeSha2UniqueSecret(SecretNum msecretNum);

  /// @brief Compute SHA2 HMAC.
  /// @param[out] hmac Computed HMAC.
  MaximInterface_EXPORT error_code computeSha2Hmac(Sha256::Hash::span hmac);
};

inline error_code make_error_code(DS28C36::ErrorValue e) {
  return error_code(e, DS28C36::errorCategory());
}

/// @brief
/// Hash arbitrary length data with successive Compute Multiblock Hash commands.
/// @param ds28c36 Device for computation.
/// @param data Data to hash.
MaximInterface_EXPORT error_code
computeMultiblockHash(DS28C36 & ds28c36, span<const uint_least8_t> data);

/// @brief Verify ECDSA signature.
/// @param ds28c36 Device for computation.
/// @param publicKey Public key to use for verification.
/// @param data Data to verify.
/// @param signature Signature to verify.
/// @param pioa New state of PIOA if verification successful.
/// @param piob New state of PIOB if verification successful.
MaximInterface_EXPORT error_code verifyEcdsaSignature(
    DS28C36 & ds28c36, DS28C36::KeyNum publicKey,
    span<const uint_least8_t> data, Ecc256::Signature::const_span signature,
    DS28C36::PioState pioa = DS28C36::Unchanged,
    DS28C36::PioState piob = DS28C36::Unchanged);

/// @brief Verify ECDSA signature.
/// @param ds28c36 Device for computation.
/// @param publicKey
/// Public key to use for verification which is loaded into Public Key S.
/// @param data Data to verify.
/// @param signature Signature to verify.
/// @param pioa New state of PIOA if verification successful.
/// @param piob New state of PIOB if verification successful.
MaximInterface_EXPORT error_code verifyEcdsaSignature(
    DS28C36 & ds28c36, Ecc256::PublicKey::const_span publicKey,
    span<const uint_least8_t> data, Ecc256::Signature::const_span signature,
    DS28C36::PioState pioa = DS28C36::Unchanged,
    DS28C36::PioState piob = DS28C36::Unchanged);

/// @brief
/// Read the device ROM ID and MAN ID using the Read Memory command on the
/// ROM Options page.
/// @param ds28c36 Device to read.
/// @param[out] romId Read ROM ID valid when operation is successful.
/// @param[out] manId Read MAN ID valid when operation is successful.
MaximInterface_EXPORT error_code readRomIdAndManId(DS28C36 & ds28c36,
                                                   RomId::span romId,
                                                   ManId::span manId);

/// @brief
/// Enable coprocessor functionality on the DS2476 by writing to the
/// GPIO Control page.
MaximInterface_EXPORT error_code enableCoprocessor(DS2476 & ds2476);

/// @brief
/// Disable blocking of the ROM ID on the DS2476 by writing to the
/// ROM Options page.
MaximInterface_EXPORT error_code enableRomId(DS2476 & ds2476);

/// Format page authentication input data.
class DS28C36::PageAuthenticationData {
public:
  typedef array_span<uint_least8_t,
                     RomId::size + 2 * Page::size + 1 + ManId::size>
      Result;

  PageAuthenticationData() : result_() {}

  /// Formatted data result.
  Result::const_span result() const { return result_; }

  /// @name ROM ID
  /// @brief 1-Wire ROM ID of the device.
  /// @{
  
  /// Get mutable ROM ID.
  RomId::span romId() {
    return make_span(result_).subspan<romIdIdx, RomId::size>();
  }

  /// Get immutable ROM ID.
  RomId::const_span romId() const {
    return const_cast<PageAuthenticationData &>(*this).romId();
  }

  /// Set ROM ID.
  PageAuthenticationData & setRomId(RomId::const_span romId) {
    copy(romId, this->romId());
    return *this;
  }

  /// Set ROM ID for use in anonymous mode.
  MaximInterface_EXPORT PageAuthenticationData & setAnonymousRomId();
  
  /// @}

  /// @name Page
  /// @brief Data from a device memory page.
  /// @{

  /// Get mutable page.
  Page::span page() {
    return make_span(result_).subspan<pageIdx, Page::size>();
  }

  /// Get immutable page.
  Page::const_span page() const {
    return const_cast<PageAuthenticationData &>(*this).page();
  }

  /// Set page.
  PageAuthenticationData & setPage(Page::const_span page) {
    copy(page, this->page());
    return *this;
  }
  
  /// @}

  /// @name Challenge
  /// @brief Random challenge used to prevent replay attacks.
  /// @{
  
  /// Get mutable Challenge.
  Page::span challenge() {
    return make_span(result_).subspan<challengeIdx, Page::size>();
  }

  /// Get immutable Challenge.
  Page::const_span challenge() const {
    return const_cast<PageAuthenticationData &>(*this).challenge();
  }

  /// Set Challenge.
  PageAuthenticationData & setChallenge(Page::const_span challenge) {
    copy(challenge, this->challenge());
    return *this;
  }
  
  /// @}

  /// @name Page number
  /// @brief Number of the page to use data from.
  /// @{
  
  /// Get page number.
  int pageNum() const { return result_[pageNumIdx]; }

  /// Set page number.
  PageAuthenticationData & setPageNum(int pageNum) {
    result_[pageNumIdx] = pageNum;
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
    return const_cast<PageAuthenticationData &>(*this).manId();
  }

  /// Set MAN ID.
  PageAuthenticationData & setManId(ManId::const_span manId) {
    copy(manId, this->manId());
    return *this;
  }
  
  /// @}

private:
  typedef Result::span::index_type index;

  static const index romIdIdx = 0;
  static const index pageIdx = romIdIdx + RomId::size;
  static const index challengeIdx = pageIdx + Page::size;
  static const index pageNumIdx = challengeIdx + Page::size;
  static const index manIdIdx = pageNumIdx + 1;

  Result::array result_;
};

/// Format authenticated write input data.
class DS28C36::WriteAuthenticationData {
public:
  typedef PageAuthenticationData::Result Result;

  WriteAuthenticationData() : data() {}

  /// Formatted data result.
  Result::const_span result() const { return data.result(); }

  /// @name ROM ID
  /// @brief 1-Wire ROM ID of the device.
  /// @{
  
  /// Get mutable ROM ID.
  RomId::span romId() { return data.romId(); }

  /// Get immutable ROM ID.
  RomId::const_span romId() const { return data.romId(); }

  /// Set ROM ID.
  WriteAuthenticationData & setRomId(RomId::const_span romId) {
    data.setRomId(romId);
    return *this;
  }

  /// Set ROM ID for use in anonymous mode.
  WriteAuthenticationData & setAnonymousRomId() {
    data.setAnonymousRomId();
    return *this;
  }
  
  /// @}

  /// @name Old page
  /// @brief Existing data contained in the page.
  /// @{
  
  /// Get mutable old page.
  Page::span oldPage() { return data.page(); }

  /// Get immutable old page.
  Page::const_span oldPage() const { return data.page(); }

  /// Set old page.
  WriteAuthenticationData & setOldPage(Page::const_span oldPage) {
    data.setPage(oldPage);
    return *this;
  }
  
  /// @}

  /// @name New page
  /// @brief New data to write to the page.
  /// @{
  
  /// Get mutable new page.
  Page::span newPage() { return data.challenge(); }

  /// Get immutable new page.
  Page::const_span newPage() const { return data.challenge(); }

  /// Set new page.
  WriteAuthenticationData & setNewPage(Page::const_span newPage) {
    data.setChallenge(newPage);
    return *this;
  }
  
  /// @}

  /// @name Page number
  /// @brief Page number for write operation.
  /// @{

  /// Get page number.
  int pageNum() const { return data.pageNum(); }

  /// Set page number.
  WriteAuthenticationData & setPageNum(int pageNum) {
    data.setPageNum(pageNum);
    return *this;
  }
  
  /// @}

  /// @name MAN ID
  /// @brief Manufacturer ID of the device.
  /// @{

  /// Get mutable MAN ID.
  ManId::span manId() { return data.manId(); }

  /// Get immutable MAN ID.
  ManId::const_span manId() const { return data.manId(); }

  /// Set MAN ID.
  WriteAuthenticationData & setManId(ManId::const_span manId) {
    data.setManId(manId);
    return *this;
  }
  
  /// @}

private:
  PageAuthenticationData data;
};

/// Format compute secret input data.
class DS28C36::ComputeSecretData {
public:
  typedef PageAuthenticationData::Result Result;

  ComputeSecretData() : data() {}

  /// Formatted data result.
  Result::const_span result() const { return data.result(); }

  /// @name ROM ID
  /// @brief 1-Wire ROM ID of the device.
  /// @{
  
  /// Get mutable ROM ID.
  RomId::span romId() { return data.romId(); }

  /// Get immutable ROM ID.
  RomId::const_span romId() const { return data.romId(); }

  /// Set ROM ID.
  ComputeSecretData & setRomId(RomId::const_span romId) {
    data.setRomId(romId);
    return *this;
  }
  
  /// @}

  /// @name Binding Data
  /// @brief Binding Data contained in the selected page.
  /// @{
  
  /// Get mutable Binding Data.
  Page::span bindingData() { return data.page(); }

  /// Get immutable Binding Data.
  Page::const_span bindingData() const { return data.page(); }

  /// Set Binding Data.
  ComputeSecretData & setBindingData(Page::const_span bindingData) {
    data.setPage(bindingData);
    return *this;
  }
  
  /// @}

  /// @name Partial Secret
  /// @brief Partial Secret used for customization.
  /// @{
  
  /// Get mutable Partial Secret.
  Page::span partialSecret() { return data.challenge(); }

  /// Get immutable Partial Secret.
  Page::const_span partialSecret() const { return data.challenge(); }

  /// Set Partial Secret.
  ComputeSecretData & setPartialSecret(Page::const_span partialSecret) {
    data.setChallenge(partialSecret);
    return *this;
  }
  
  /// @}

  /// @name Page number
  /// @brief Page number for Binding Data.
  /// @{
  
  /// Get page number.
  int pageNum() const { return data.pageNum(); }

  /// Set page number.
  ComputeSecretData & setPageNum(int pageNum) {
    data.setPageNum(pageNum);
    return *this;
  }
  
  /// @}

  /// @name MAN ID
  /// @brief Manufacturer ID of the device.
  /// @{
  
  /// Get mutable MAN ID.
  ManId::span manId() { return data.manId(); }

  /// Get immutable MAN ID.
  ManId::const_span manId() const { return data.manId(); }

  /// Set MAN ID.
  ComputeSecretData & setManId(ManId::const_span manId) {
    data.setManId(manId);
    return *this;
  }
  
  /// @}

private:
  PageAuthenticationData data;
};

/// Format encryption or decryption HMAC input data.
class DS28C36::EncryptionHmacData {
public:
  typedef array_span<uint_least8_t,
                     EncryptionChallenge::size + RomId::size + 1 + ManId::size>
      Result;

  EncryptionHmacData() : result_() {}

  /// Formatted data result.
  Result::const_span result() const { return result_; }

  /// @name Encryption Challenge
  /// @brief Random challenge used to prevent replay attacks.
  /// @{
  
  /// Get mutable Encryption Challenge.
  EncryptionChallenge::span encryptionChallenge() {
    return make_span(result_)
        .subspan<encryptionChallengeIdx, EncryptionChallenge::size>();
  }

  /// Get immutable Encryption Challenge.
  EncryptionChallenge::const_span encryptionChallenge() const {
    return const_cast<EncryptionHmacData &>(*this).encryptionChallenge();
  }

  /// Set Encryption Challenge.
  EncryptionHmacData &
  setEncryptionChallenge(EncryptionChallenge::const_span encryptionChallenge) {
    copy(encryptionChallenge, this->encryptionChallenge());
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
    return const_cast<EncryptionHmacData &>(*this).romId();
  }

  /// Set ROM ID.
  EncryptionHmacData & setRomId(RomId::const_span romId) {
    copy(romId, this->romId());
    return *this;
  }

  /// Set ROM ID for use in anonymous mode.
  MaximInterface_EXPORT EncryptionHmacData & setAnonymousRomId();
  
  /// @}

  /// @name Page number
  /// @brief Number of the page to use data from.
  /// @{
  
  /// Get page number.
  int pageNum() const { return result_[pageNumIdx]; }

  /// Set page number.
  EncryptionHmacData & setPageNum(int pageNum) {
    result_[pageNumIdx] = pageNum;
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
    return const_cast<EncryptionHmacData &>(*this).manId();
  }

  /// Set MAN ID.
  EncryptionHmacData & setManId(ManId::const_span manId) {
    copy(manId, this->manId());
    return *this;
  }
  
  /// @}

private:
  typedef Result::span::index_type index;

  static const index encryptionChallengeIdx = 0;
  static const index romIdIdx =
      encryptionChallengeIdx + EncryptionChallenge::size;
  static const index pageNumIdx = romIdIdx + RomId::size;
  static const index manIdIdx = pageNumIdx + 1;

  Result::array result_;
};

/// Access fields in the ROM Options page.
class DS28C36::RomOptions {
public:
  explicit RomOptions(Page::span page) : page(page) {}

  bool romBlockDisable() const {
    return page[romBlockDisableIdx] == enabledValue;
  }
  
  RomOptions & setRomBlockDisable(bool romBlockDisable) {
    page[romBlockDisableIdx] = (romBlockDisable ? enabledValue : 0);
    return *this;
  }
  
  bool anonymous() const { return page[anonymousIdx] == enabledValue; }

  RomOptions & setAnonymous(bool anonymous) {
    page[anonymousIdx] = (anonymous ? enabledValue : 0);
    return *this;
  }

  ManId::const_span manId() const {
    return page.subspan<22, ManId::size>();
  }

  RomId::const_span romId() const {
    return page.subspan<24, RomId::size>();
  }

private:
  static const Page::span::index_type romBlockDisableIdx = 0;
  static const Page::span::index_type anonymousIdx = 1;
  static const Page::span::value_type enabledValue = 0xAA;

  Page::span page;
};

/// Access fields in the GPIO Control page.
class DS28C36::GpioControl {
public:
  explicit GpioControl(Page::span page) : page(page) {}

  bool pioaConducting() const {
    return page[pioaConductingIdx] == pioConductingValue;
  }

  GpioControl & setPioaConducting(bool pioaConducting) {
    page[pioaConductingIdx] = (pioaConducting ? pioConductingValue : 0x55);
    return *this;
  }

  bool piobConducting() const {
    return page[piobConductingIdx] == pioConductingValue;
  }

  GpioControl & setPiobConducting(bool piobConducting) {
    page[piobConductingIdx] = (piobConducting ? pioConductingValue : 0x55);
    return *this;
  }

  bool pioaLevel() const { return page[2] == pioLevelValue; }

  bool piobLevel() const { return page[3] == pioLevelValue; }

private:
  static const Page::span::index_type pioaConductingIdx = 0;
  static const Page::span::index_type piobConductingIdx = 1;
  static const Page::span::value_type pioConductingValue = 0xAA;
  static const Page::span::value_type pioLevelValue = 0x55;

  Page::span page;
};

} // namespace MaximInterface

#endif
