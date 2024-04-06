#pragma once

#include <stdint.h>

/// @file
///
/// _minstd0_ random number generator.

/// Random number generator using the _minstd0_ algorithm.
static inline uint32_t minstd_gen(uint32_t* state) {
    uint64_t p = ((uint64_t)*state) * 48271;
    uint32_t x = (p & 0x7fffffff) + (p >> 31);
    return *state = (x & 0x7fffffff) + (x >> 31);
}
