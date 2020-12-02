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

#include <memory>

namespace atomic_ops {
enum memory_order : int {
  acquire = __ATOMIC_ACQUIRE,
  relaxed = __ATOMIC_RELAXED,
  release = __ATOMIC_RELEASE,
  acq_rel = __ATOMIC_ACQ_REL,
  seq_cst = __ATOMIC_SEQ_CST
};

template <typename T>
constexpr T load(T& t, memory_order order) {
  return __atomic_load_n(std::addressof(t), order);
}

template <typename T, typename U>
constexpr void store(T& t, U u, memory_order order) {
  __atomic_store_n(std::addressof(t), u, order);
}

template <typename T, typename U>
constexpr T fetch_add(T& t, U u, memory_order order) {
  return __atomic_fetch_add(std::addressof(t), u, order);
}

template <typename T, typename U>
constexpr T fetch_and(T& t, U u, memory_order order) {
  return __atomic_fetch_and(std::addressof(t), u, order);
}

template <typename T, typename U>
constexpr T fetch_or(T& t, U u, memory_order order) {
  return __atomic_fetch_or(std::addressof(t), u, order);
}

enum MemoryModel {
  SequentialConsistency,
  ReleaseConsistency,
  RelaxedConsistency,
  Unsynchronized
};

template <MemoryModel mm>
struct mm_tag {
  constexpr operator MemoryModel() const {
    return mm;
  }
};

inline constexpr mm_tag<SequentialConsistency> sc = {};
inline constexpr mm_tag<ReleaseConsistency>    rc = {};
inline constexpr mm_tag<RelaxedConsistency>    xc = {};
inline constexpr mm_tag<Unsynchronized>        unsync = {};

static constexpr memory_order load_order(MemoryModel mm) {
  switch (mm) {
   case SequentialConsistency: return seq_cst;
   case ReleaseConsistency:    return acquire;
   case RelaxedConsistency:    return relaxed;
   case Unsynchronized:        return relaxed;
  };
  __builtin_unreachable();
}

static constexpr memory_order store_order(MemoryModel mm) {
  switch (mm) {
   case SequentialConsistency: return seq_cst;
   case ReleaseConsistency:    return release;
   case RelaxedConsistency:    return relaxed;
   case Unsynchronized:        return relaxed;
  };
  __builtin_unreachable();
}

static constexpr memory_order rmw_order(MemoryModel mm) {
  switch (mm) {
   case SequentialConsistency: return seq_cst;
   case ReleaseConsistency:    return acq_rel;
   case RelaxedConsistency:    return relaxed;
   case Unsynchronized:        return relaxed;
  };
  __builtin_unreachable();
}

template <MemoryModel mm, typename T>
constexpr T load(T& t, mm_tag<mm> = {}) {
  if constexpr (mm == Unsynchronized) {
    return t;
  }
  else {
    return load(t, load_order(mm));
  }
}

template <MemoryModel mm, typename T, typename U>
void store(T& t, U u, mm_tag<mm> = {}) {
  if constexpr (mm == Unsynchronized) {
    t = u;
  }
  else {
    store(t, u, store_order(mm));
  }
}

template <MemoryModel mm, typename T, typename U>
T fetch_add(T& t, U u, mm_tag<mm> = {}) {
  if constexpr (mm == Unsynchronized) {
    T temp = t;
    t = temp + u;
    return temp;
  }
  else {
    return fetch_add(t, u, rmw_order(mm));
  }
}

template <MemoryModel mm, typename T, typename U>
T fetch_and(T& t, U u, mm_tag<mm> = {}) {
  if constexpr (mm == Unsynchronized) {
    T temp = t;
    t = temp & u;
    return temp;
  }
  else {
    return fetch_and(t, u, rmw_order(mm));
  }
}

template <MemoryModel mm, typename T, typename U>
T fetch_or(T& t, U u, mm_tag<mm> = {}) {
  if constexpr (mm == Unsynchronized) {
    T temp = t;
    t = temp | u;
    return temp;
  }
  else {
    return fetch_or(t, u, rmw_order(mm));
  }
}
}
