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

#ifndef MaximInterface_Segment
#define MaximInterface_Segment

#include <iterator>
#include <utility>
#include "type_traits.hpp"

namespace MaximInterface {

/// @brief
/// Advances a given iterator by a given number of elements with bounds checking.
/// @tparam InputIt Must meet the requirements of InputIterator.
/// @param[in,out] it Iterator to advance. 
/// @param bound
/// Past-the-end boundary iterator. If distance is positive, bound must be
/// reachable by incrementing the given iterator. If distance is negative, bound
/// must be reachable by decrementing the given iterator.
/// @param distance
/// Number of elements to advance the given iterator. If distance is positive,
/// the given iterator is incremented. If distance is negative, the given
/// iterator is decremented, and InputIt must meet the requirements of
/// BidirectionalIterator.
/// @returns The number of elements that the given iterator was advanced.
template <typename InputIt>
typename std::iterator_traits<InputIt>::difference_type checkedAdvance(
    InputIt & it, const InputIt bound,
    typename std::iterator_traits<InputIt>::difference_type distance) {
  typedef
      typename std::iterator_traits<InputIt>::difference_type difference_type;

  // Use constant-time operations if InputIt is a random access iterator.
  if (is_same<typename std::iterator_traits<InputIt>::iterator_category,
              std::random_access_iterator_tag>::value) {
    const difference_type boundDistance = std::distance(it, bound);
    if (boundDistance >= 0) {
      if (distance > boundDistance) {
        distance = boundDistance;
      } else if (distance < 0) {
        distance = 0;
      }
    } else {
      if (distance < boundDistance) {
        distance = boundDistance;
      } else if (distance > 0) {
        distance = 0;
      }
    }
    std::advance(it, distance);
  } else {
    const difference_type startingDistance = distance;
    while (distance != 0 && it != bound) {
      if (distance > 0) {
        ++it;
        --distance;
      } else {
        --it;
        ++distance;
      }
    }
    if (startingDistance > 0) {
      distance = startingDistance - distance;
    } else {
      distance = startingDistance + distance;
    }
  }
  return distance;
}

/// @brief Locates an iterator sub-range using segment number addressing.
/// @details
/// Useful for devices that divide the memory space into uniform chunks such as
/// pages and segments.
/// @tparam ForwardIt Must meet the requirements of ForwardIterator.
/// @param begin Beginning of the input data range.
/// @param end End of the input data range.
/// @param segmentSize Number of elements contained in a segment.
/// @param segmentNum Zero-indexed number of the desired segment.
/// @returns
/// Pair of iterators representing the sub-range of the segment within
/// the input range. If the segment does not exist within the input range, both
/// iterators in the pair are set to the end iterator of the input range.
template <typename ForwardIt, typename Index>
std::pair<ForwardIt, ForwardIt> createSegment(
    ForwardIt begin, const ForwardIt end,
    const typename std::iterator_traits<ForwardIt>::difference_type segmentSize,
    Index segmentNum) {
  ForwardIt segmentEnd = begin;
  typename std::iterator_traits<ForwardIt>::difference_type lastSegmentSize =
      checkedAdvance(segmentEnd, end, segmentSize);
  while (segmentNum > 0 && segmentEnd != end) {
    begin = segmentEnd;
    lastSegmentSize = checkedAdvance(segmentEnd, end, segmentSize);
    --segmentNum;
  }
  if (segmentNum > 0 || lastSegmentSize != segmentSize) {
    begin = segmentEnd;
  }
  return std::make_pair(begin, segmentEnd);
}

} // namespace MaximInterface

#endif