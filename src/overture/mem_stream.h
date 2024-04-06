#pragma once

#include <stddef.h>
#include <stdio.h>

/**
 * @file
 *
 * Memory stream that provides a `FILE*` object that can be used to write to memory. On POSIX
 * systems, this uses the ability to specify a custom read/write function on `FILE*` objects. On
 * other systems, this might require to create a temporary file.
 */

/// Memory stream object.
struct mem_stream {
    FILE* file;     ///< File stream, suitable for writing operations.
    char* buf;      ///< Buffer containing the data that has been written to the file object.
    size_t size;    ///< Buffer size, in bytes.
};

/// Initializes a memory stream.
void mem_stream_init(struct mem_stream*);
/// Destroys a memory stream. 
/// @warning The buffer of the memory stream must be released via `free()` manually.
void mem_stream_destroy(struct mem_stream*);
/// Flushes the contents written so far to the memory stream buffer.
void mem_stream_flush(struct mem_stream*);
