#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include "str.h"

/// @file
///
/// File manipulation functions.

/// Reads the contents of a file into memory.
/// @param file_name Name of the file on disk.
/// @param size On return, contains the size of the resulting buffer, excluding the `NULL` terminator. May be `NULL`.
/// @return A `NULL`-terminated buffer with the contents of the file, or `NULL` if reading fails. Must be freed using `free()` by the caller.
[[nodiscard]] char* read_file(const char* file_name, size_t* total_size);

/// @return `true` if the given file exists, otherwise `false`.
[[nodiscard]] bool file_exists(const char* file_name);

/// @return The file size in bytes, or 0 if the file does not exist.
[[nodiscard]] size_t file_size(const char* file_name);

/// @return A full path to the given file name, or NULL. Must be freed by the caller using `free`.
[[nodiscard]] char* full_path(const char* file_name);

/// @return `true` if the given character is a path separator, otherwise `false`.
[[nodiscard]] bool is_path_sep(char);

/// File path components.
struct file_path {
    struct str_view dir_name;
    struct str_view base_name;
    struct str_view ext;
};

/// Splits a file name into its directory, base name, and extension parts.
[[nodiscard]] struct file_path split_path(struct str_view file_name);
