#pragma once

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mem.h"

/// @file
///
/// Bit manipulation functions.

static_assert(sizeof(uint64_t) == sizeof(double));
static_assert(sizeof(uint32_t) == sizeof(float));

/// Makes a bitmask with the given number of lower bits set.
[[nodiscard]] static inline uint64_t make_bitmask(size_t bits) {
    assert(bits <= 64);
    return (bits == 64 ? 0 : (UINT64_C(1) << bits)) - 1;
}

/// Sign-extends the given value to the given number of bits.
[[nodiscard]] static inline uint64_t sign_extend(uint64_t val, size_t bit_count) {
    const uint64_t mask = UINT64_C(1) << (bit_count - 1);
    return (val ^ mask) - mask;
}

/// Converts a double-precision floating-point number to an unsigned integer by reinterpreting its
/// bit pattern.
[[nodiscard]] static inline uint64_t double_to_bits(double x) {
    uint64_t y;
    xmemcpy(&y, &x, sizeof(x));
    return y;
}

/// Converts a single-precision floating-point number to an unsigned integer by reinterpreting its
/// bit pattern.
[[nodiscard]] static inline uint32_t float_to_bits(float x) {
    uint32_t y;
    xmemcpy(&y, &x, sizeof(x));
    return y;
}

/// Converts an unsigned integer to a double-precision floating-point number by reinterpreting its
/// bit pattern.
[[nodiscard]] static inline double bits_to_double(uint64_t x) {
    double y;
    xmemcpy(&y, &x, sizeof(x));
    return y;
}

/// Converts an unsigned integer to a single-precision floating-point number by reinterpreting its
/// bit pattern.
[[nodiscard]] static inline float bits_to_float(uint32_t x) {
    float y;
    xmemcpy(&y, &x, sizeof(x));
    return y;
}
