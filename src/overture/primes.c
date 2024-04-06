#include "primes.h"

#include <assert.h>

size_t next_prime(size_t i) {
#define f(x) if (i <= x) return x;
    PRIMES(f)
    return i;
#undef f
}

size_t mod_prime(size_t i, size_t p) {
    assert(p != 0);
    switch (p) {
#define f(x) case x: return i % x;
    PRIMES(f)
#undef f
        default: return i % p;
    }
}
