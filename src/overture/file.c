#include "file.h"
#include "mem.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

char* read_file(const char* file_name, size_t* total_size) {
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

bool file_exists(const char* file_name) {
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

size_t file_size(const char* file_name) {
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

char* full_path(const char* file_name) {
#if _XOPEN_SOURCE >= 500 || _DEFAULT_SOURCE || _BSD_SOURCE
    return realpath(file_name, NULL);
#elif WIN32
    return _fullpath(NULL, file_name, 0);
#else
#warning "Missing implementation for `realpath()` -- Reported file paths will be incorrect"
    return strdup(file_name);
#endif
}

bool is_path_sep(char c) {
    return
#if WIN32
        c == '\\' ||
#endif
        c == '/';
}

struct file_path split_path(struct str_view file_name) {
    size_t dir_end   = 0;
    size_t ext_begin = file_name.length;
    for (size_t i = file_name.length; i-- > 0; ) {
        if (is_path_sep(file_name.data[i])) {
            dir_end = i;
            break;
        }

        if (file_name.data[i] == '.')
            ext_begin = i;
    }

    return (struct file_path) {
        .dir_name  = { .data = file_name.data,               .length = dir_end },
        .base_name = { .data = file_name.data + dir_end + 1, .length = ext_begin - dir_end - 1 },
        .ext       = { .data = file_name.data + ext_begin,   .length = file_name.length - ext_begin },
    };
}
