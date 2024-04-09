#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/// @file
///
/// Safer memory-related routines.

/// Stops the program with the given message.
// GCOV_EXCL_START
[[noreturn]] static inline void die(const char* msg) {
    fputs(msg, stderr);
    abort();
}
// GCOV_EXCL_STOP

/// Allocates data on the heap using `malloc()`. Prints an error and aborts on failure.
[[nodiscard]] static inline void* xmalloc(size_t size) {
    void* p = malloc(size);
    if (!p)
        die("out of memory, malloc() failed.\n");
    return p;
}

/// Allocates data on the heap using `calloc()`. Prints an error and aborts on failure.
[[nodiscard]] static inline void* xcalloc(size_t count, size_t size) {
    void* p = calloc(count, size);
    if (!p)
        die("out of memory, calloc() failed.\n");
    return p;
}

/// Reallocates data on the heap using `realloc()`. Prints an error and aborts on failure. Unlike
/// `realloc()`, this function allows reallocating with a size of 0.
[[nodiscard]] static inline void* xrealloc(void* p, size_t size) {
    if (size == 0) {
        free(p);
        return xmalloc(0);
    }
    p = realloc(p, size);
    if (!p)
        die("out of memory, realloc() failed.\n");
    return p;
}

/// Copies memory to the destination, just like `memcpy()`. Unlike `memcpy()`, this function allows
/// `NULL` pointers in the source or destination if the number of bytes to copy is 0.
static inline void* xmemcpy(void* restrict dest, const void* restrict src, size_t count) {
    if (count == 0)
        return dest;
    assert(dest);
    assert(src);
    return memcpy(dest, src, count);
}
