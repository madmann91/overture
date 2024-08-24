#include "file.h"
#include "mem.h"

#include <stdlib.h>

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

char* file_read(const char* file_name, size_t* total_size) {
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
