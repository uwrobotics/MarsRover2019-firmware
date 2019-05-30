/*******************************************************************************
* Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
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

#ifndef MaximInterface_span
#define MaximInterface_span

#include <stddef.h>
#include <algorithm>
#include <iterator>
#include <vector>
#include "array.hpp"
#include "type_traits.hpp"

namespace MaximInterface {

static const ptrdiff_t dynamic_extent = -1;

struct with_container_t {
  explicit with_container_t(int) {}
};

static const with_container_t with_container(0);

namespace detail {

template <template <typename, ptrdiff_t = MaximInterface::dynamic_extent>
          class span,
          typename T, ptrdiff_t Extent>
class span_base {
public:
  typedef T element_type;
  typedef typename remove_cv<element_type>::type value_type;
  typedef ptrdiff_t index_type;
  typedef ptrdiff_t difference_type;
  typedef element_type * pointer;
  typedef element_type & reference;
  typedef element_type * iterator;
  typedef const element_type * const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  static const index_type extent = Extent;

protected:
  span_base(pointer data) : data_(data) {}
  ~span_base() {}

public:
  /// @name Iterators
  /// @{
  
  iterator begin() const {
    return const_cast<iterator>(static_cast<const span_base &>(*this).cbegin());
  }
  
  const_iterator cbegin() const { return data(); }
  
  iterator end() const {
    return const_cast<iterator>(static_cast<const span_base &>(*this).cend());
  }
  
  const_iterator cend() const { return cbegin() + size(); }
  
  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }
  
  reverse_iterator rend() const { return reverse_iterator(begin()); }
  
  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }
  
  /// @}

  /// @name Element access
  /// @{
  
  reference operator[](index_type idx) const { return data()[idx]; }
  
  reference operator()(index_type idx) const { return operator[](idx); }
  
  pointer data() const { return data_; }
  
  /// @}

  /// @name Subviews
  /// @{
  
  template <index_type Count> span<element_type, Count> first() const {
    return subspan<0, Count>();
  }
  
  span<element_type> first(index_type Count) const { return subspan(0, Count); }
  
  span<element_type> last(index_type Count) const {
    return subspan(size() - Count, Count);
  }
  
  template <index_type Offset, index_type Count>
  span<element_type, Count> subspan() const {
    return span<element_type, Count>(data() + Offset, Count);
  }
  
  span<element_type> subspan(index_type Offset,
                             index_type Count = dynamic_extent) const {
    return span<element_type>(
        data() + Offset, Count == dynamic_extent ? size() - Offset : Count);
  }
  
  /// @}

private:
  index_type size() const {
    return static_cast<const span<T, Extent> &>(*this).size();
  }

  pointer data_;
};

} // namespace detail

/// Generic memory span class similar to gsl::span or the proposed std::span.
template <typename T, ptrdiff_t Extent = dynamic_extent>
class span : public detail::span_base<MaximInterface::span, T, Extent> {
  typedef detail::span_base<MaximInterface::span, T, Extent> span_base;

public:
  using span_base::extent;
  using typename span_base::element_type;
  using typename span_base::index_type;
  using typename span_base::pointer;
  using typename span_base::value_type;

  span(pointer data, index_type) : span_base(data) {}
  
  span(pointer begin, pointer) : span_base(begin) {}
  
  span(element_type (&arr)[extent]) : span_base(arr) {}
  
  span(array<value_type, extent> & arr) : span_base(arr.data()) {}
  
  span(const array<value_type, extent> & arr) : span_base(arr.data()) {}
  
  template <typename U> span(const span<U, extent> & s) : span_base(s.data()) {}
  
  template <typename Allocator>
  explicit span(std::vector<value_type, Allocator> & vec)
      : span_base(&vec.front()) {}
      
  template <typename Allocator>
  explicit span(const std::vector<value_type, Allocator> & vec)
      : span_base(&vec.front()) {}
      
  template <typename Container>
  span(with_container_t, Container & cont) : span_base(cont.data()) {}
  
  template <typename Container>
  span(with_container_t, const Container & cont) : span_base(cont.data()) {}

  /// @name Observers
  /// @{
  
  static index_type size() { return extent; }
  
  static index_type size_bytes() { return size() * sizeof(element_type); }
  
  static bool empty() { return size() == 0; }
  
  /// @}

  /// @name Subviews
  /// @{
  
  template <index_type Count> span<element_type, Count> last() const {
    return this->template subspan<extent - Count, Count>();
  }
  
  /// @}
};

template <typename T>
class span<T, dynamic_extent>
    : public detail::span_base<MaximInterface::span, T, dynamic_extent> {
  typedef detail::span_base<MaximInterface::span, T, dynamic_extent> span_base;

public:
  using typename span_base::element_type;
  using typename span_base::index_type;
  using typename span_base::pointer;
  using typename span_base::value_type;

  span() : span_base(NULL), size_(0) {}
  
  span(pointer data, index_type size) : span_base(data), size_(size) {}
  
  span(pointer begin, pointer end) : span_base(begin), size_(end - begin) {}
  
  template <size_t N> span(element_type (&arr)[N]) : span_base(arr), size_(N) {}
  
  template <size_t N>
  span(array<value_type, N> & arr) : span_base(arr.data()), size_(N) {}
  
  template <size_t N>
  span(const array<value_type, N> & arr) : span_base(arr.data()), size_(N) {}
  
  template <typename U, ptrdiff_t N>
  span(const span<U, N> & s) : span_base(s.data()), size_(s.size()) {}
  
  template <typename Allocator>
  span(std::vector<value_type, Allocator> & vec)
      : span_base(vec.empty() ? NULL : &vec.front()), size_(vec.size()) {}
      
  template <typename Allocator>
  span(const std::vector<value_type, Allocator> & vec)
      : span_base(vec.empty() ? NULL : &vec.front()), size_(vec.size()) {}
      
  template <typename Container>
  span(with_container_t, Container & cont)
      : span_base(cont.data()), size_(cont.size()) {}
      
  template <typename Container>
  span(with_container_t, const Container & cont)
      : span_base(cont.data()), size_(cont.size()) {}

  /// @name Observers
  /// @{
  
  index_type size() const { return size_; }
  
  index_type size_bytes() const { return size() * sizeof(element_type); }
  
  bool empty() const { return size() == 0; }
  
  /// @}

  /// @name Subviews
  /// @{
  
  template <index_type Count> span<element_type, Count> last() const {
    return span<element_type, Count>(this->data() + (size() - Count), Count);
  }
  
  /// @}

private:
  index_type size_;
};

template <typename T, ptrdiff_t Extent, typename U>
bool operator==(span<T, Extent> lhs, span<U, Extent> rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, ptrdiff_t Extent, typename U>
bool operator!=(span<T, Extent> lhs, span<U, Extent> rhs) {
  return !operator==(lhs, rhs);
}

template <typename T, ptrdiff_t Extent, typename U>
bool operator<(span<T, Extent> lhs, span<U, Extent> rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end());
}

template <typename T, ptrdiff_t Extent, typename U>
bool operator>(span<T, Extent> lhs, span<U, Extent> rhs) {
  return operator<(rhs, lhs);
}

template <typename T, ptrdiff_t Extent, typename U>
bool operator<=(span<T, Extent> lhs, span<U, Extent> rhs) {
  return !operator>(lhs, rhs);
}

template <typename T, ptrdiff_t Extent, typename U>
bool operator>=(span<T, Extent> lhs, span<U, Extent> rhs) {
  return !operator<(lhs, rhs);
}

template <typename T>
span<T> make_span(T * data, typename span<T>::index_type size) {
  return span<T>(data, size);
}

template <typename T> span<T> make_span(T * begin, T * end) {
  return span<T>(begin, end);
}

template <typename T, size_t N> span<T, N> make_span(T (&arr)[N]) {
  return span<T, N>(arr);
}

template <typename T, size_t N> span<T, N> make_span(array<T, N> & arr) {
  return arr;
}

template <typename T, size_t N>
span<const T, N> make_span(const array<T, N> & arr) {
  return arr;
}

template <typename T, typename Allocator>
span<T> make_span(std::vector<T, Allocator> & vec) {
  return vec;
}

template <typename T, typename Allocator>
span<const T> make_span(const std::vector<T, Allocator> & vec) {
  return vec;
}

template <typename Container>
span<typename Container::value_type> make_span(with_container_t,
                                               Container & cont) {
  return span<typename Container::value_type>(with_container, cont);
}

template <typename Container>
span<const typename Container::value_type> make_span(with_container_t,
                                                     const Container & cont) {
  return span<const typename Container::value_type>(with_container, cont);
}

/// Deep copy between static spans of the same size.
template <typename T, ptrdiff_t Extent, typename U>
typename enable_if<Extent != dynamic_extent>::type copy(span<T, Extent> src,
                                                        span<U, Extent> dst) {
  std::copy(src.begin(), src.end(), dst.begin());
}

} // namespace MaximInterface

#endif
