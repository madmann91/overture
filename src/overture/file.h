#pragma once

#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

/// Reads the contents of a file into memory.
/// @param file_name Name of the file on disk.
/// @param size On return, contains the size of the resulting buffer, excluding the `NULL` terminator. May be `NULL`.
/// @return A `NULL`-terminated buffer that must be freed using `free()` by the caller.
[[nodiscard]] static inline char* file_read(const char* file_name, size_t* total_size) {
    FILE* file = fopen(file_name, "rb");
    if (!file)
        return NULL;

    size_t capacity = 1024;
    char* data = xmalloc(capacity);
    size_t size = 0;

    while (true) {
        size_t to_read = capacity - size;
        if (to_read == 0) {
            capacity += capacity >> 1;
            data = xrealloc(data, capacity);
            to_read = capacity - size;
        }
        size_t read = fread(data + size, 1, to_read, file);
        size += read;
        if (read < to_read)
            break;
    }
    fclose(file);

    data = xrealloc(data, size + 1);
    data[size] = 0;
    if (total_size)
        *total_size = size;
    return data;
}

/// @return `true` if the given file exists, otherwise `false`.
[[nodiscard]] static inline bool file_exists(const char* file_name) {
#ifdef _WIN32
    FILE* file = fopen(file_name, "rb");
    if (!file)
        return false;
    fclose(file);
    return true;
#else
    return access(file_name, F_OK) == 0;
#endif
}

/// @return The file size in bytes, or 0 if the file does not exist.
[[nodiscard]] static inline size_t file_size(const char* file_name) {
#ifdef _WIN32
    FILE* file = fopen(file_name, "rb");
    if (!file)
        return 0;
    fseek(file, 0, SEEK_END);
    long pos = ftell(f);
    fclose(file);
    return pos;
#else
    struct stat st;
    if (stat(file_name, &st) != 0)
        return 0;
    return st.st_size;
#endif
}
