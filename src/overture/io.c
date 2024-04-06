#include "io.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
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

bool is_terminal(FILE* file) {
    return isatty(fileno(file));
}
