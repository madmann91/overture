#pragma once

#include <stddef.h>
#include <assert.h>

/**
 * @file 
 *
 * List of primes specifically designed for hash tables.
 */

#define MIN_PRIME 7
#define MAX_PRIME 1048583

/// List of primes. Can be instantiated with a given function macro like so:
///
///     #define f(x) x,
///     size_t primes[] = {
///         PRIMES(f)
///     };
///     #undef f
///
#define PRIMES(f) \
    f(MIN_PRIME) \
    f(17) \
    f(31) \
    f(67) \
    f(257) \
    f(1031) \
    f(4093) \
    f(8191) \
    f(16381) \
    f(32381) \
    f(65539) \
    f(131071) \
    f(262147) \
    f(524287) \
    f(MAX_PRIME)

/// Produces the next prime on the list after the given number. If there is no such prime, this
/// function returns the given value unchanged.
[[nodiscard]] static inline size_t next_prime(size_t i) {
#define f(x) if (i <= x) return x;
    PRIMES(f)
    return i;
#undef f
}

/// Computes the remainder of the division of the given value by the given divisor. If the divisor
/// is not a prime from the list of primes @ref PRIMES, this function still works, albeit a bit slower.
[[nodiscard]] static inline size_t mod_prime(size_t i, size_t p) {
    assert(p != 0);
    switch (p) {
#define f(x) case x: return i % x;
    PRIMES(f)
#undef f
        default: return i % p;
    }
}
