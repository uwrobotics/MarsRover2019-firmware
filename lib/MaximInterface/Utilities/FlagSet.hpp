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

#ifndef MaximInterface_FlagSet
#define MaximInterface_FlagSet

#include <stddef.h>
#include <bitset>

namespace MaximInterface {

/// @brief
/// Provides functionality similar to std::bitset except using a bit flag,
/// typically of an enum type, as the indexer.
template <typename Flag, size_t flagBits> class FlagSet {
public:
  class reference {
  public:
    reference(FlagSet & flagSet, Flag flag) : flagSet(&flagSet), flag(flag) {}
    
    reference & operator=(bool x) {
      flagSet->set(flag, x);
      return *this;
    }
    
    reference & operator=(const reference & x) {
      return operator=(static_cast<bool>(x));
    }
    
    operator bool() const { return flagSet->test(flag); }
    
    bool operator~() const { return reference(*this).flip(); }
    
    reference & flip() {
      *this = !*this;
      return *this;
    }

  private:
    FlagSet * flagSet;
    Flag flag;
  };

  FlagSet() : bits() {}
  
  FlagSet(unsigned long val) : bits(val) {}

  template <typename CharT, typename Traits, typename Alloc>
  explicit FlagSet(
      const std::basic_string<CharT, Traits, Alloc> & str,
      typename std::basic_string<CharT, Traits, Alloc>::size_type pos = 0,
      typename std::basic_string<CharT, Traits, Alloc>::size_type n =
          std::basic_string<CharT, Traits, Alloc>::npos)
      : bits(str, pos, n) {}

  bool operator==(const FlagSet & rhs) const { return bits == rhs.bits; }
  
  bool operator!=(const FlagSet & rhs) const { return !operator==(rhs); }

  /// @name Element access
  /// @{
  
  bool operator[](Flag flag) const { return test(flag); }
  
  reference operator[](Flag flag) { return reference(*this, flag); }
  
  bool test(Flag flag) const { return (bits.to_ulong() & flag) == flag; }
  
  bool any() const { return bits.any(); }
  
  bool none() const { return bits.none(); }
  
  size_t count() const { return bits.count(); }
  
  /// @}

  /// @name Capacity
  /// @{
  
  size_t size() const { return bits.size(); }
  
  /// @}

  /// @name Modifiers
  /// @{
  
  FlagSet & operator&=(const FlagSet & other) {
    bits &= other.bits;
    return *this;
  }
  
  FlagSet & operator|=(const FlagSet & other) {
    bits |= other.bits;
    return *this;
  }
  
  FlagSet & operator^=(const FlagSet & other) {
    bits ^= other.bits;
    return *this;
  }
  
  FlagSet operator~() const { return ~bits; }

  FlagSet & set() {
    bits.set();
    return *this;
  }
  
  FlagSet & set(Flag flag, bool value = true) {
    if (value) {
      bits |= flag;
    } else {
      bits &= ~std::bitset<flagBits>(flag);
    }
    return *this;
  }
  
  FlagSet & reset() {
    bits.reset();
    return *this;
  }
  
  FlagSet & reset(Flag flag) { return set(flag, false); }
  
  FlagSet & flip() {
    bits.flip();
    return *this;
  }
  
  FlagSet & flip(Flag flag) {
    bits ^= flag;
    return *this;
  }
  
  /// @}

  /// @name Conversions
  /// @{
  
  template <typename CharT, typename Traits, typename Allocator>
  std::basic_string<CharT, Traits, Allocator> to_string() const {
    return bits.template to_string<CharT, Traits, Allocator>();
  }
  
  unsigned long to_ulong() const { return bits.to_ulong(); }
  
  /// @}

private:
  std::bitset<flagBits> bits;

  template <typename CharT, typename Traits>
  friend std::basic_ostream<CharT, Traits> &
  operator<<(std::basic_ostream<CharT, Traits> & os, const FlagSet & x);

  template <typename CharT, class Traits>
  friend std::basic_istream<CharT, Traits> &
  operator>>(std::basic_istream<CharT, Traits> & is, FlagSet & x);
};

template <typename Flag, size_t flagBits>
FlagSet<Flag, flagBits> operator&(const FlagSet<Flag, flagBits> & lhs,
                                  const FlagSet<Flag, flagBits> & rhs) {
  return FlagSet<Flag, flagBits>(lhs) &= rhs;
}

template <typename Flag, size_t flagBits>
FlagSet<Flag, flagBits> operator|(const FlagSet<Flag, flagBits> & lhs,
                                  const FlagSet<Flag, flagBits> & rhs) {
  return FlagSet<Flag, flagBits>(lhs) |= rhs;
}

template <typename Flag, size_t flagBits>
FlagSet<Flag, flagBits> operator^(const FlagSet<Flag, flagBits> & lhs,
                                  const FlagSet<Flag, flagBits> & rhs) {
  return FlagSet<Flag, flagBits>(lhs) ^= rhs;
}

template <typename CharT, typename Traits, typename Flag, size_t flagBits>
std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> & os,
           const FlagSet<Flag, flagBits> & x) {
  os << x.bits;
  return os;
}

template <typename CharT, class Traits, typename Flag, size_t flagBits>
std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> & is,
           FlagSet<Flag, flagBits> & x) {
  is >> x.bits;
  return is;
}

} // namespace MaximInterface

#endif
