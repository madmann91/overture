#include "mem_stream.h"
#include "alloc.h"

#include <assert.h>
#include <stdlib.h>

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L
#define ENABLE_MEMSTREAM
#endif

void mem_stream_init(struct mem_stream* mem_stream) {
    mem_stream->size = 0;
    mem_stream->buf = NULL;
#ifdef ENABLE_MEMSTREAM
    mem_stream->file = open_memstream(&mem_stream->buf, &mem_stream->size);
#else
    mem_stream->file = tmpfile();
#endif
    assert(mem_stream->file);
}

void mem_stream_destroy(struct mem_stream* mem_stream) {
    mem_stream_flush(mem_stream);
    fclose(mem_stream->file);
    mem_stream->file = NULL;
}

void mem_stream_flush(struct mem_stream* mem_stream) {
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
