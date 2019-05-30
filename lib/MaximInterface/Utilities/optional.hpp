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

#ifndef MaximInterface_optional
#define MaximInterface_optional

#include "SafeBool.hpp"

// Include for std::swap.
#include <algorithm>
#include <utility>

namespace MaximInterface {

struct nullopt_t {
  explicit nullopt_t(int) {}
};

static const nullopt_t nullopt(0);

/// @brief Optional value container similar to std::optional.
/// @details
/// To prevent the need for aligned storage, this implementation imposes that
/// types must be DefaultConstructible, CopyConstructible, and CopyAssignable.
/// No exceptions are thrown when accessing a valueless optional.
template <typename T> class optional {
public:
  typedef T value_type;

  optional() : value_(), has_value_(false) {}
  
  optional(nullopt_t) : value_(), has_value_(false) {}

  optional(const T & value) : value_(value), has_value_(true) {}

  template <typename U>
  explicit optional(const optional<U> & other)
      : value_(other.value_), has_value_(other.has_value_) {}

  optional & operator=(nullopt_t) {
    reset();
    return *this;
  }

  optional & operator=(const T & value) {
    value_ = value;
    has_value_ = true;
    return *this;
  }

  template <typename U> optional & operator=(const optional<U> & other) {
    if (has_value_ || other.has_value_) {
      value_ = other.value_;
      has_value_ = other.has_value_;
    }
    return *this;
  }

  bool has_value() const { return has_value_; }
  
  operator SafeBool() const { return makeSafeBool(has_value()); }

  const T & value() const { return value_; }
  
  T & value() {
    return const_cast<T &>(static_cast<const optional &>(*this).value());
  }
  
  const T & operator*() const { return value(); }
  
  T & operator*() {
    return const_cast<T &>(static_cast<const optional &>(*this).operator*());
  }
  
  const T * operator->() const { return &value(); }
  
  T * operator->() {
    return const_cast<T *>(static_cast<const optional &>(*this).operator->());
  }

  const T & value_or(const T & default_value) const {
    return has_value() ? value() : default_value;
  }

  void swap(optional & other) {
    if (has_value_ || other.has_value_) {
      using std::swap;
      swap(value_, other.value_);
      swap(has_value_, other.has_value_);
    }
  }

  void reset() {
    if (has_value_) {
      has_value_ = false;
      value_ = T();
    }
  }

private:
  T value_;
  bool has_value_;
};

template <typename T> optional<T> make_optional(const T & value) {
  return value;
}

template <typename T> void swap(optional<T> & lhs, optional<T> & rhs) {
  lhs.swap(rhs);
}

template <typename T, typename U>
bool operator==(const optional<T> & lhs, const optional<U> & rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }
  if (!lhs.has_value()) {
    return true;
  }
  return lhs.value() == rhs.value();
}

template <typename T, typename U>
bool operator!=(const optional<T> & lhs, const optional<U> & rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return true;
  }
  if (!lhs.has_value()) {
    return false;
  }
  return lhs.value() != rhs.value();
}

template <typename T, typename U>
bool operator<(const optional<T> & lhs, const optional<U> & rhs) {
  if (!rhs.has_value()) {
    return false;
  }
  if (!lhs.has_value()) {
    return true;
  }
  return lhs.value() < rhs.value();
}

template <typename T, typename U>
bool operator<=(const optional<T> & lhs, const optional<U> & rhs) {
  if (!lhs.has_value()) {
    return true;
  }
  if (!rhs.has_value()) {
    return false;
  }
  return lhs.value() <= rhs.value();
}

template <typename T, typename U>
bool operator>(const optional<T> & lhs, const optional<U> & rhs) {
  if (!lhs.has_value()) {
    return false;
  }
  if (!rhs.has_value()) {
    return true;
  }
  return lhs.value() > rhs.value();
}

template <typename T, typename U>
bool operator>=(const optional<T> & lhs, const optional<U> & rhs) {
  if (!rhs.has_value()) {
    return true;
  }
  if (!lhs.has_value()) {
    return false;
  }
  return lhs.value() >= rhs.value();
}

template <typename T> bool operator==(const optional<T> & opt, nullopt_t) {
  return !opt.has_value();
}

template <typename T> bool operator==(nullopt_t, const optional<T> & opt) {
  return operator==(opt, nullopt);
}

template <typename T> bool operator!=(const optional<T> & opt, nullopt_t) {
  return !operator==(opt, nullopt);
}

template <typename T> bool operator!=(nullopt_t, const optional<T> & opt) {
  return operator!=(opt, nullopt);
}

template <typename T> bool operator<(const optional<T> &, nullopt_t) {
  return false;
}

template <typename T> bool operator<(nullopt_t, const optional<T> & opt) {
  return opt.has_value();
}

template <typename T> bool operator<=(const optional<T> & opt, nullopt_t) {
  return !operator>(opt, nullopt);
}

template <typename T> bool operator<=(nullopt_t, const optional<T> & opt) {
  return !operator>(nullopt, opt);
}

template <typename T> bool operator>(const optional<T> & opt, nullopt_t) {
  return operator<(nullopt, opt);
}

template <typename T> bool operator>(nullopt_t, const optional<T> & opt) {
  return operator<(opt, nullopt);
}

template <typename T> bool operator>=(const optional<T> & opt, nullopt_t) {
  return !operator<(opt, nullopt);
}

template <typename T> bool operator>=(nullopt_t, const optional<T> & opt) {
  return !operator<(nullopt, opt);
}

template <typename T, typename U>
bool operator==(const optional<T> & opt, const U & value) {
  return opt.has_value() ? opt.value() == value : false;
}

template <typename T, typename U>
bool operator==(const T & value, const optional<U> & opt) {
  return operator==(opt, value);
}

template <typename T, typename U>
bool operator!=(const optional<T> & opt, const U & value) {
  return opt.has_value() ? opt.value() != value : true;
}

template <typename T, typename U>
bool operator!=(const T & value, const optional<U> & opt) {
  return operator!=(opt, value);
}

template <typename T, typename U>
bool operator<(const optional<T> & opt, const U & value) {
  return opt.has_value() ? opt.value() < value : true;
}

template <typename T, typename U>
bool operator<(const T & value, const optional<U> & opt) {
  return opt.has_value() ? value < opt.value() : false;
}

template <typename T, typename U>
bool operator<=(const optional<T> & opt, const U & value) {
  return opt.has_value() ? opt.value() <= value : true;
}

template <typename T, typename U>
bool operator<=(const T & value, const optional<U> & opt) {
  return opt.has_value() ? value <= opt.value() : false;
}

template <typename T, typename U>
bool operator>(const optional<T> & opt, const U & value) {
  return opt.has_value() ? opt.value() > value : false;
}

template <typename T, typename U>
bool operator>(const T & value, const optional<U> & opt) {
  return opt.has_value() ? value > opt.value() : true;
}

template <typename T, typename U>
bool operator>=(const optional<T> & opt, const U & value) {
  return opt.has_value() ? opt.value() >= value : false;
}

template <typename T, typename U>
bool operator>=(const T & value, const optional<U> & opt) {
  return opt.has_value() ? value >= opt.value() : true;
}

} // namespace MaximInterface

#endif
