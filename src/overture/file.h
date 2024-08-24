#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

/// @file
///
/// File manipulation functions.

/// Reads the contents of a file into memory.
/// @param file_name Name of the file on disk.
/// @param size On return, contains the size of the resulting buffer, excluding the `NULL` terminator. May be `NULL`.
/// @return A `NULL`-terminated buffer with the contents of the file. Must be freed using `free()` by the caller.
[[nodiscard]] char* file_read(const char* file_name, size_t* total_size);

/// @return `true` if the given file exists, otherwise `false`.
[[nodiscard]] bool file_exists(const char* file_name);

/// @return The file size in bytes, or 0 if the file does not exist.
[[nodiscard]] size_t file_size(const char* file_name);
