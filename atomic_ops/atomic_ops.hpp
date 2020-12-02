#pragma once

#include <memory>

namespace atomic {
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
