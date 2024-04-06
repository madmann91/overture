#pragma once

#include "bits.h"

#include <stdint.h>

/// @file
///
/// FNV-1a hash function, for bytes, words, double words, quad words, floating-point numbers, and
/// strings.

[[nodiscard]] static inline uint32_t hash_init(void) {
    return 0x811c9dc5;
}

[[nodiscard]] static inline uint32_t hash_uint8(uint32_t h, uint8_t x) {
    return (h ^ x) * 0x01000193;
}

[[nodiscard]] static inline uint32_t hash_uint16(uint32_t h, uint16_t x) {
    return hash_uint8(hash_uint8(h, x >> 8), x);
}

[[nodiscard]] static inline uint32_t hash_uint32(uint32_t h, uint32_t x) {
    return hash_uint16(hash_uint16(h, x >> 16), x);
}

[[nodiscard]] static inline uint32_t hash_uint64(uint32_t h, uint64_t x) {
    return hash_uint32(hash_uint32(h, x >> 32), x);
}

[[nodiscard]] static inline uint32_t hash_float(uint32_t h, float x) {
    return hash_uint32(h, float_to_bits(x));
}

[[nodiscard]] static inline uint32_t hash_double(uint32_t h, double x) {
    return hash_uint64(h, double_to_bits(x));
}

[[nodiscard]] static inline uint32_t hash_string(uint32_t h, const char* str) {
    for (; *str; str++)
        h = hash_uint8(h, *str);
    return h;
}
