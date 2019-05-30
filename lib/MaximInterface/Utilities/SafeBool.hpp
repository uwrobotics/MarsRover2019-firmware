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

#ifndef MaximInterface_SafeBool
#define MaximInterface_SafeBool

#include <stddef.h>

namespace MaximInterface {
namespace detail {

// Implement SafeBool as a member function pointer since some compilers allow
// function pointer to void pointer conversion.
struct SafeBoolHelper {
  void trueValue() const {}
  
private:
  SafeBoolHelper(); // deleted
};

} // namespace detail

/// @brief Type definition for SafeBool.
/// @details
/// SafeBool is a boolean type that eliminates many error-prone implicit
/// conversions allowed by the fundamental bool type.
/// @note
/// SafeBool can be used to add an explicit bool conversion operator to a type.
typedef void (detail::SafeBoolHelper::*SafeBool)() const;

/// Constructor for SafeBool.
inline SafeBool makeSafeBool(bool value) {
  return value ? &detail::SafeBoolHelper::trueValue : NULL;
}

} // namespace MaximInterface

#endif
