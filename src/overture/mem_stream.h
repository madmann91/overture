#pragma once

#include "alloc.h"

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/// @file
///
/// Memory stream that provides a `FILE*` object that can be used to write to memory. On POSIX
/// systems, this uses the ability to specify a custom read/write function on `FILE*` objects. On
/// other systems, this might require a temporary file.

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L
#define ENABLE_MEMSTREAM
#endif

/// Memory stream object.
struct mem_stream {
    FILE* file;     ///< File stream, suitable for writing operations.
    char* buf;      ///< Buffer containing the data that has been written to the file object.
    size_t size;    ///< Buffer size, in bytes.
};

/// Initializes a memory stream.
static inline void mem_stream_init(struct mem_stream* mem_stream) {
    mem_stream->size = 0;
    mem_stream->buf = NULL;
#ifdef ENABLE_MEMSTREAM
    mem_stream->file = open_memstream(&mem_stream->buf, &mem_stream->size);
#else
    mem_stream->file = tmpfile();
#endif
    assert(mem_stream->file);
}

/// Flushes the contents written so far to the memory stream buffer.
static inline void mem_stream_flush(struct mem_stream* mem_stream) {
#ifndef ENABLE_MEMSTREAM
    long pos = ftell(mem_stream->file);
    rewind(mem_stream->file);
    mem_stream->buf = xrealloc(mem_stream->buf, pos + 1);
    mem_stream->size = pos;
    fread(mem_stream->buf, 1, pos, mem_stream->file);
    fseek(mem_stream->file, pos, SEEK_SET);
    mem_stream->buf[pos] = 0;
#else
    fflush(mem_stream->file);
#endif
}

/// Destroys a memory stream.
/// @warning The buffer of the memory stream must be released via `free()` manually.
static inline void mem_stream_destroy(struct mem_stream* mem_stream) {
    mem_stream_flush(mem_stream);
    fclose(mem_stream->file);
    mem_stream->file = NULL;
}
