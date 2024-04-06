#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

/// Reads the contents of a file into memory.
/// @param file_name Name of the file on disk.
/// @param size On return, contains the size of the resulting buffer, excluding the `NULL` terminator. May be `NULL`.
/// @return A `NULL`-terminated buffer that must be freed using `free()` by the caller.
[[nodiscard]] char* read_file(const char* file_name, size_t* size);
/// @return `true` if the given stream is a terminal, `false` otherwise.
/// This may be used as a way to detect when to turn on/off ANSI color codes in the output.
[[nodiscard]] bool is_terminal(FILE*);
