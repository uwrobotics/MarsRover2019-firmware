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

#ifndef MaximInterface_ChangeSizeType
#define MaximInterface_ChangeSizeType

#include <stdint.h>
#include <algorithm>
#include <limits>
#include "span.hpp"
#include "system_error.hpp"

namespace MaximInterface {

/// @brief
/// Adapts functions taking an array pointer and size where the size type is
/// nonstandard.
/// @tparam NewSize Nonstandard size type.
/// @tparam Func Must be callable as func(Data *, NewSize).
/// @tparam Data Array element type.
/// @param func Callback for processing chunks of data.
/// @param data Data to process with callback.
template <typename NewSize, typename Func, typename Data>
error_code changeSizeType(Func func, span<Data> data) {
  using namespace std;
  typedef typename span<Data>::index_type DataSize;

  error_code result;
  // Check if NewSize can represent the maximum value of DataSize.
  if (static_cast<uintmax_t>(numeric_limits<DataSize>::max()) >
      static_cast<uintmax_t>(numeric_limits<NewSize>::max())) {
    DataSize dataIdx = 0;
    do {
      const span<Data> chunk =
          data.subspan(dataIdx, min<DataSize>(data.size() - dataIdx,
                                              numeric_limits<NewSize>::max()));
      result = func(chunk.data(), static_cast<NewSize>(chunk.size()));
      dataIdx += chunk.size();
    } while (dataIdx < data.size() && !result);
  } else {
    result = func(data.data(), static_cast<NewSize>(data.size()));
  }
  return result;
}

} // namespace MaximInterface

#endif