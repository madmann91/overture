#include <overture/test.h>
#include <overture/mem_stream.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TEST(mem_stream) {
    struct mem_stream mem_stream;
    mem_stream_init(&mem_stream);
    fprintf(mem_stream.file, "%s %s!", "Hello", "world");
    mem_stream_flush(&mem_stream);
    const char* text = "Hello world!";
    REQUIRE(strncmp(mem_stream.buf, text, strlen(text)) == 0);
    mem_stream_destroy(&mem_stream);
    free(mem_stream.buf);
}
