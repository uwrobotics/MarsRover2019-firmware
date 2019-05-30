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

#include "Error.hpp"
#include "system_error.hpp"

namespace MaximInterface {

using std::string;

error_condition error_category::default_error_condition(int code) const {
  return error_condition(code, *this);
}

bool error_category::equivalent(int code,
                                const error_condition & condition) const {
  return default_error_condition(code) == condition;
}

bool error_category::equivalent(const error_code & code, int condition) const {
  return *this == code.category() && code.value() == condition;
}

const error_category & system_category() {
  static class : public error_category {
  public:
    virtual const char * name() const { return "system"; }

    virtual string message(int condition) const {
      return defaultErrorMessage(condition);
    }
  } instance;
  return instance;
}

static string formatWhat(const error_code & ec) {
  return string(ec.category().name()) + ' ' + ec.message();
}

template <typename T>
static string formatWhat(const error_code & ec, const T & what_arg) {
  return string(what_arg) + ": " + formatWhat(ec);
}

system_error::system_error(const error_code & ec)
    : runtime_error(formatWhat(ec)), code_(ec) {}

system_error::system_error(const error_code & ec, const string & what_arg)
    : runtime_error(formatWhat(ec, what_arg)), code_(ec) {}
    
system_error::system_error(const error_code & ec, const char * what_arg)
    : runtime_error(formatWhat(ec, what_arg)), code_(ec) {}

system_error::system_error(int ev, const error_category & ecat)
    : runtime_error(formatWhat(error_code(ev, ecat))), code_(ev, ecat) {}

system_error::system_error(int ev, const error_category & ecat,
                           const string & what_arg)
    : runtime_error(formatWhat(error_code(ev, ecat), what_arg)),
      code_(ev, ecat) {}
      
system_error::system_error(int ev, const error_category & ecat,
                           const char * what_arg)
    : runtime_error(formatWhat(error_code(ev, ecat), what_arg)),
      code_(ev, ecat) {}

} // namespace MaximInterface
