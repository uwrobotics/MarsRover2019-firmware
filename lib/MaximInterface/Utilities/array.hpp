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

#ifndef MaximInterface_array
#define MaximInterface_array

#include <stddef.h>
#include <stdint.h>
#include <algorithm>
#include <iterator>

namespace MaximInterface {

/// Generic array class similar to std::array.
template <typename T, size_t N> class array {
public:
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef value_type & reference;
  typedef const value_type & const_reference;
  typedef value_type * pointer;
  typedef const value_type * const_pointer;
  typedef pointer iterator;
  typedef const_pointer const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  /// @name Element access
  /// @{
  
  reference operator[](size_type pos) {
    return const_cast<reference>(
        static_cast<const array &>(*this).operator[](pos));
  }
  
  const_reference operator[](size_type pos) const { return data()[pos]; }
  
  reference front() {
    return const_cast<reference>(static_cast<const array &>(*this).front());
  }
  
  const_reference front() const { return operator[](0); }
  
  reference back() {
    return const_cast<reference>(static_cast<const array &>(*this).back());
  }
  
  const_reference back() const { return operator[](size() - 1); }
  
  pointer data() {
    return const_cast<pointer>(static_cast<const array &>(*this).data());
  }
  
  const_pointer data() const { return _buffer; }
  
  /// @}

  /// @name Iterators
  /// @{
  
  iterator begin() {
    return const_cast<iterator>(static_cast<const array &>(*this).cbegin());
  }
  
  const_iterator begin() const { return cbegin(); }
  
  const_iterator cbegin() const { return data(); }
  
  iterator end() {
    return const_cast<iterator>(static_cast<const array &>(*this).cend());
  }
  
  const_iterator end() const { return cend(); }
  
  const_iterator cend() const { return cbegin() + size(); }
  
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  
  const_reverse_iterator crbegin() const { return rbegin(); }
  
  reverse_iterator rend() { return reverse_iterator(begin()); }
  
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  
  const_reverse_iterator crend() const { return rend(); }
  
  /// @}

  /// @name Capacity
  /// @{
  
  static bool empty() { return size() == 0; }
  
  static size_type size() { return N; }
  
  static size_type max_size() { return size(); }
  
  /// Alternative to size() when a constant expression is required.
  static const size_type csize = N;
  
  /// @}

  /// @name Operations
  /// @{
  
  void fill(const_reference value) { std::fill(begin(), end(), value); }
  
  void swap(array & other) { std::swap_ranges(begin(), end(), other.begin()); }
  
  /// @}

  /// @private
  /// @note Implementation detail set public to allow aggregate initialization.
  T _buffer[N];
};

template <typename T, size_t N>
inline bool operator==(const array<T, N> & lhs, const array<T, N> & rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, size_t N>
inline bool operator!=(const array<T, N> & lhs, const array<T, N> & rhs) {
  return !operator==(lhs, rhs);
}

template <typename T, size_t N>
inline bool operator<(const array<T, N> & lhs, const array<T, N> & rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end());
}

template <typename T, size_t N>
inline bool operator>(const array<T, N> & lhs, const array<T, N> & rhs) {
  return operator<(rhs, lhs);
}

template <typename T, size_t N>
inline bool operator<=(const array<T, N> & lhs, const array<T, N> & rhs) {
  return !operator>(lhs, rhs);
}

template <typename T, size_t N>
inline bool operator>=(const array<T, N> & lhs, const array<T, N> & rhs) {
  return !operator<(lhs, rhs);
}

template <typename T, size_t N>
inline void swap(array<T, N> & lhs, array<T, N> & rhs) {
  lhs.swap(rhs);
}

} // namespace MaximInterface

#endif
