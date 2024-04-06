#pragma once

#include <stddef.h>

/// Allocates data on the heap using `malloc`. Prints an error and aborts on failure.
[[nodiscard]] void* xmalloc(size_t);
/// Allocates data on the heap using `calloc`. Prints an error and aborts on failure.
[[nodiscard]] void* xcalloc(size_t, size_t);
/// Reallocates data on the heap using `realloc`. Prints an error and aborts on failure.
[[nodiscard]] void* xrealloc(void*, size_t);
