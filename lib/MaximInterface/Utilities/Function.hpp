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

#ifndef MaximInterface_Function
#define MaximInterface_Function

#include <stddef.h>
#include "SafeBool.hpp"
#include "type_traits.hpp"

// Include for std::swap.
#include <algorithm>
#include <utility>

namespace MaximInterface {
namespace detail {

// Provides char buffer storage for a given type.
// Suitability of alignment for type should be verified with alignment_of.
template <typename Type, size_t TypeSize> union TypeStorage {
public:
  operator const Type *() const { return reinterpret_cast<const Type *>(data); }

  operator Type *() {
    return const_cast<Type *>(
        static_cast<const Type *>(static_cast<const TypeStorage &>(*this)));
  }

  const Type & operator*() const { return **this; }

  Type & operator*() {
    return const_cast<Type &>(
        static_cast<const TypeStorage &>(*this).operator*());
  }

  const Type * operator->() const { return *this; }

  Type * operator->() {
    return const_cast<Type *>(
        static_cast<const TypeStorage &>(*this).operator->());
  }

private:
  char data[TypeSize];
  long double aligner1;
  long int aligner2;
  void * aligner3;
};

// Computes the internal target size for TypeStorage based on a desired total
// size. No internal storage will be allocated if the requested size is smaller
// than the required data elements of TypeWrapper or if the requested size is
// smaller than minimum size of TypeStorage.
template <typename Target, size_t totalSize> class TypeWrapperTotalSize {
private:
  typedef TypeStorage<Target, 1> MinSizeStorage;
  
  static const size_t otherDataSize = sizeof(Target *);
  
  // Round down to be a multiple of alignment_of<MinSizeTargetStorage>::value.
  static const size_t internalTargetRawSize =
      (totalSize - otherDataSize) / alignment_of<MinSizeStorage>::value *
      alignment_of<MinSizeStorage>::value;

public:
  // Use internal storage if internalTargetRawSize is valid and at least as
  // large as the minimum size of TypeStorage.
  static const size_t internalTargetSize =
      ((totalSize > otherDataSize) &&
       (internalTargetRawSize >= sizeof(MinSizeStorage)))
          ? internalTargetRawSize
          : 0;
};

// Basic type erasure implementation with small object optimization.
template <typename Target, template <typename> class TargetAdapter,
          size_t internalTargetSize =
              TypeWrapperTotalSize<Target, 32>::internalTargetSize>
class TypeWrapper {
private:
  typedef TypeStorage<Target, internalTargetSize> TargetStorage;

public:
  TypeWrapper() : currentTarget(NULL) {}

  TypeWrapper(const TypeWrapper & other) {
    if (other.currentTarget == other.internalTarget) {
      other.currentTarget->clone(internalTarget);
      currentTarget = internalTarget;
    } else if (other.currentTarget) {
      currentTarget = other.currentTarget->clone();
    } else {
      currentTarget = NULL;
    }
  }

  template <typename Source> TypeWrapper(Source source) {
    if (sizeof(TargetAdapter<Source>) <= internalTargetSize &&
        alignment_of<TargetAdapter<Source> >::value <=
            alignment_of<TargetStorage>::value) {
      new (internalTarget) TargetAdapter<Source>(source);
      currentTarget = internalTarget;
    } else {
      currentTarget = new TargetAdapter<Source>(source);
    }
  }

  ~TypeWrapper() {
    if (currentTarget == internalTarget) {
      currentTarget->~Target();
    } else {
      delete currentTarget;
    }
  }

  const TypeWrapper & operator=(const TypeWrapper & rhs) {
    TypeWrapper(rhs).swap(*this);
    return *this;
  }

  template <typename Source> const TypeWrapper & operator=(Source source) {
    TypeWrapper(source).swap(*this);
    return *this;
  }

  void clear() { TypeWrapper().swap(*this); }

  void swap(TypeWrapper & other) {
    if (this == &other) {
      return;
    }

    if (currentTarget == internalTarget &&
        other.currentTarget == other.internalTarget) {
      TargetStorage temp;
      currentTarget->clone(temp);
      currentTarget->~Target();
      currentTarget = NULL;
      other.currentTarget->clone(internalTarget);
      other.currentTarget->~Target();
      other.currentTarget = NULL;
      currentTarget = internalTarget;
      temp->clone(other.internalTarget);
      temp->~Target();
      other.currentTarget = other.internalTarget;
    } else if (currentTarget == internalTarget) {
      currentTarget->clone(other.internalTarget);
      currentTarget->~Target();
      currentTarget = other.currentTarget;
      other.currentTarget = other.internalTarget;
    } else if (other.currentTarget == other.internalTarget) {
      other.currentTarget->clone(internalTarget);
      other.currentTarget->~Target();
      other.currentTarget = currentTarget;
      currentTarget = internalTarget;
    } else {
      using std::swap;
      swap(currentTarget, other.currentTarget);
    }
  }

  const Target * target() const { return currentTarget; }

private:
  TargetStorage internalTarget;
  Target * currentTarget;
};

// TypeWrapper specialization with no internal storage space.
template <typename Target, template <typename> class TargetAdapter>
class TypeWrapper<Target, TargetAdapter, 0> {
public:
  TypeWrapper() : currentTarget(NULL) {}

  TypeWrapper(const TypeWrapper & other) {
    if (other.currentTarget) {
      currentTarget = other.currentTarget->clone();
    } else {
      currentTarget = NULL;
    }
  }

  template <typename Source>
  TypeWrapper(Source source)
      : currentTarget(new TargetAdapter<Source>(source)) {}

  ~TypeWrapper() { delete currentTarget; }

  const TypeWrapper & operator=(const TypeWrapper & rhs) {
    TypeWrapper(rhs).swap(*this);
    return *this;
  }

  template <typename Source> const TypeWrapper & operator=(Source source) {
    TypeWrapper(source).swap(*this);
    return *this;
  }

  void clear() { TypeWrapper().swap(*this); }

  void swap(TypeWrapper & other) {
    using std::swap;
    swap(currentTarget, other.currentTarget);
  }

  const Target * target() const { return currentTarget; }

private:
  Target * currentTarget;
};

} // namespace detail

// Function wrapper similar to std::function for 0-3 argument functions.
template <typename> class Function;

// Function implementation for zero argument functions.
template <typename ResultType> class Function<ResultType()> {
public:
  typedef ResultType result_type;

  Function(ResultType (*func)() = NULL) : callableWrapper() {
    if (func) {
      callableWrapper = func;
    }
  }

  template <typename F> Function(F func) : callableWrapper(func) {}

  const Function & operator=(ResultType (*func)()) {
    if (func) {
      callableWrapper = func;
    } else {
      callableWrapper.clear();
    }
    return *this;
  }

  template <typename F> const Function & operator=(F func) {
    callableWrapper = func;
    return *this;
  }

  void swap(Function & other) { callableWrapper.swap(other.callableWrapper); }

  operator SafeBool() const {
    return makeSafeBool(callableWrapper.target() != NULL);
  }

  ResultType operator()() const {
    return callableWrapper.target() ? callableWrapper.target()->invoke()
                                    : ResultType();
  }

private:
  class Callable {
  public:
    virtual ~Callable() {}
    virtual ResultType invoke() const = 0;
    virtual Callable * clone() const = 0;
    virtual void clone(void * buffer) const = 0;
  };

  template <typename F> class CallableAdapter : public Callable {
  public:
    CallableAdapter(F func) : func(func) {}

    virtual ResultType invoke() const { return func(); }

    virtual Callable * clone() const { return new CallableAdapter(*this); }

    virtual void clone(void * buffer) const {
      new (buffer) CallableAdapter(*this);
    }

  private:
    F func;
  };

  detail::TypeWrapper<Callable, CallableAdapter> callableWrapper;
};

template <typename ResultType>
inline void swap(Function<ResultType()> & lhs, Function<ResultType()> & rhs) {
  lhs.swap(rhs);
}

// Function implementation for one argument functions.
template <typename ArgumentType, typename ResultType>
class Function<ResultType(ArgumentType)> {
public:
  typedef ArgumentType argument_type;
  typedef ResultType result_type;

  Function(ResultType (*func)(ArgumentType) = NULL) : callableWrapper() {
    if (func) {
      callableWrapper = func;
    }
  }

  template <typename F> Function(F func) : callableWrapper(func) {}

  const Function & operator=(ResultType (*func)(ArgumentType)) {
    if (func) {
      callableWrapper = func;
    } else {
      callableWrapper.clear();
    }
    return *this;
  }

  template <typename F> const Function & operator=(F func) {
    callableWrapper = func;
    return *this;
  }

  void swap(Function & other) { callableWrapper.swap(other.callableWrapper); }

  operator SafeBool() const {
    return makeSafeBool(callableWrapper.target() != NULL);
  }

  ResultType operator()(ArgumentType arg) const {
    return callableWrapper.target() ? callableWrapper.target()->invoke(arg)
                                    : ResultType();
  }

private:
  class Callable {
  public:
    virtual ~Callable() {}
    virtual ResultType invoke(ArgumentType) const = 0;
    virtual Callable * clone() const = 0;
    virtual void clone(void * buffer) const = 0;
  };

  template <typename F> class CallableAdapter : public Callable {
  public:
    CallableAdapter(F func) : func(func) {}

    virtual ResultType invoke(ArgumentType arg) const { return func(arg); }

    virtual Callable * clone() const { return new CallableAdapter(*this); }

    virtual void clone(void * buffer) const {
      new (buffer) CallableAdapter(*this);
    }

  private:
    F func;
  };

  detail::TypeWrapper<Callable, CallableAdapter> callableWrapper;
};

template <typename ArgumentType, typename ResultType>
inline void swap(Function<ResultType(ArgumentType)> & lhs,
                 Function<ResultType(ArgumentType)> & rhs) {
  lhs.swap(rhs);
}

// Function implementation for two argument functions.
template <typename FirstArgumentType, typename SecondArgumentType,
          typename ResultType>
class Function<ResultType(FirstArgumentType, SecondArgumentType)> {
public:
  typedef FirstArgumentType first_argument_type;
  typedef SecondArgumentType second_argument_type;
  typedef ResultType result_type;

  Function(ResultType (*func)(FirstArgumentType, SecondArgumentType) = NULL)
      : callableWrapper() {
    if (func) {
      callableWrapper = func;
    }
  }

  template <typename F> Function(F func) : callableWrapper(func) {}

  const Function & operator=(ResultType (*func)(FirstArgumentType,
                                                SecondArgumentType)) {
    if (func) {
      callableWrapper = func;
    } else {
      callableWrapper.clear();
    }
    return *this;
  }

  template <typename F> const Function & operator=(F func) {
    callableWrapper = func;
    return *this;
  }

  void swap(Function & other) { callableWrapper.swap(other.callableWrapper); }

  operator SafeBool() const {
    return makeSafeBool(callableWrapper.target() != NULL);
  }

  ResultType operator()(FirstArgumentType arg1, SecondArgumentType arg2) const {
    return callableWrapper.target()
               ? callableWrapper.target()->invoke(arg1, arg2)
               : ResultType();
  }

private:
  class Callable {
  public:
    virtual ~Callable() {}
    virtual ResultType invoke(FirstArgumentType, SecondArgumentType) const = 0;
    virtual Callable * clone() const = 0;
    virtual void clone(void * buffer) const = 0;
  };

  template <typename F> class CallableAdapter : public Callable {
  public:
    CallableAdapter(F func) : func(func) {}

    virtual ResultType invoke(FirstArgumentType arg1,
                              SecondArgumentType arg2) const {
      return func(arg1, arg2);
    }

    virtual Callable * clone() const { return new CallableAdapter(*this); }

    virtual void clone(void * buffer) const {
      new (buffer) CallableAdapter(*this);
    }

  private:
    F func;
  };

  detail::TypeWrapper<Callable, CallableAdapter> callableWrapper;
};

template <typename FirstArgumentType, typename SecondArgumentType,
          typename ResultType>
inline void
swap(Function<ResultType(FirstArgumentType, SecondArgumentType)> & lhs,
     Function<ResultType(FirstArgumentType, SecondArgumentType)> & rhs) {
  lhs.swap(rhs);
}

// Function implementation for three argument functions.
template <typename FirstArgumentType, typename SecondArgumentType,
          typename ThirdArgumentType, typename ResultType>
class Function<ResultType(FirstArgumentType, SecondArgumentType,
                          ThirdArgumentType)> {
public:
  typedef ResultType result_type;

  Function(ResultType (*func)(FirstArgumentType, SecondArgumentType,
                              ThirdArgumentType) = NULL)
      : callableWrapper() {
    if (func) {
      callableWrapper = func;
    }
  }

  template <typename F> Function(F func) : callableWrapper(func) {}

  const Function & operator=(ResultType (*func)(FirstArgumentType,
                                                SecondArgumentType,
                                                ThirdArgumentType)) {
    if (func) {
      callableWrapper = func;
    } else {
      callableWrapper.clear();
    }
    return *this;
  }

  template <typename F> const Function & operator=(F func) {
    callableWrapper = func;
    return *this;
  }

  void swap(Function & other) { callableWrapper.swap(other.callableWrapper); }

  operator SafeBool() const {
    return makeSafeBool(callableWrapper.target() != NULL);
  }

  ResultType operator()(FirstArgumentType arg1, SecondArgumentType arg2,
                        ThirdArgumentType arg3) const {
    return callableWrapper.target()
               ? callableWrapper.target()->invoke(arg1, arg2, arg3)
               : ResultType();
  }

private:
  class Callable {
  public:
    virtual ~Callable() {}
    virtual ResultType invoke(FirstArgumentType, SecondArgumentType,
                              ThirdArgumentType) const = 0;
    virtual Callable * clone() const = 0;
    virtual void clone(void * buffer) const = 0;
  };

  template <typename F> class CallableAdapter : public Callable {
  public:
    CallableAdapter(F func) : func(func) {}

    virtual ResultType invoke(FirstArgumentType arg1, SecondArgumentType arg2,
                              ThirdArgumentType arg3) const {
      return func(arg1, arg2, arg3);
    }

    virtual Callable * clone() const { return new CallableAdapter(*this); }

    virtual void clone(void * buffer) const {
      new (buffer) CallableAdapter(*this);
    }

  private:
    F func;
  };

  detail::TypeWrapper<Callable, CallableAdapter> callableWrapper;
};

template <typename FirstArgumentType, typename SecondArgumentType,
          typename ThirdArgumentType, typename ResultType>
inline void swap(Function<ResultType(FirstArgumentType, SecondArgumentType,
                                     ThirdArgumentType)> & lhs,
                 Function<ResultType(FirstArgumentType, SecondArgumentType,
                                     ThirdArgumentType)> & rhs) {
  lhs.swap(rhs);
}

} // namespace MaximInterface

#endif
