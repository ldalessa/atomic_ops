// BSD 3-Clause License
//
// Copyright (c) 2020, Luke D'Alessandro
// Copyright (c) 2020, Trustees of Indiana University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include "atomic_ops.hpp"
#include <bit>

namespace atomic_ops {
static constexpr int BITMAP_WORD_BITS = 8 * sizeof(unsigned long);

template <typename T>
unsigned long bitmap_mask(T t) {
  return 1ul << t;
}

template <typename T>
T bitmap_words(T n) {
  auto w = n / BITMAP_WORD_BITS;
  auto b = n % BITMAP_WORD_BITS;
  return w + ((b) ? 1 : 0);
}

/// Atomically get a bit.
template <MemoryModel MM = ReleaseConsistency, typename T>
unsigned long bitmap_get(const unsigned long* bits, T i, mm_tag<MM> mm = {}) {
  auto w = i / BITMAP_WORD_BITS;
  auto b = i % BITMAP_WORD_BITS;
  auto m = bitmap_mask(b);
  return load(bits[w], mm) & m;
}

/// Atomically set a bit.
///
/// Returns the previous value of the bit.
template <MemoryModel MM = ReleaseConsistency, typename T>
unsigned long bitmap_set(unsigned long* bits, T i, mm_tag<MM> mm = {}) {
  auto w = i / BITMAP_WORD_BITS;
  auto b = i % BITMAP_WORD_BITS;
  auto m = bitmap_mask(b);
  return fetch_or(bits[w], m, mm) & m;
}

/// Atomically clear a bit in a bitmap.
///
/// Returns the previous value of the bit.
template <MemoryModel MM = ReleaseConsistency, typename T>
unsigned long bitmap_clear(unsigned long* bits, T i, mm_tag<MM> mm = {}) {
  auto w = i / BITMAP_WORD_BITS;
  auto b = i % BITMAP_WORD_BITS;
  auto m = ~bitmap_mask(b);
  return fetch_and(bits[w], m, mm) & m;
}

/// Find the index of the next non-zero in the bitmap.
template <MemoryModel MM = ReleaseConsistency, typename T>
T bitmap_next(const unsigned long* bits, T i, T e, mm_tag<MM> mm = {}) {
  if (unlikely(++i >= e)) {
    return e;                                   // saturate at e
  }

  auto w = i / BITMAP_WORD_BITS;
  auto b = i % BITMAP_WORD_BITS;
  auto d = load(bits[w], mm) >> b;

  if (likely(d)) {
    auto n = i + std::countr_zero(d);
    return (n < e) ? n : e;                 // saturate at e (avoid <algorithm>)
  }

  // -1 because next ++i
  return bitmap_next(bits, i + (BITMAP_WORD_BITS - b) - 1, e, mm);
}

/// First the first non-zero in the bitmap
template <MemoryModel MM = ReleaseConsistency, typename T>
T bitmap_first(const unsigned long* bits, T i, T e, mm_tag<MM> mm = {}) {
  return (bitmap_get(bits, i, mm)) ? i : bitmap_next(bits, i, e, mm);
}
}
