#include <overture/test_macros.h>
#include <overture/mem_stream.h>
#include <overture/log.h>

#include <string.h>

TEST(log) {
    struct mem_stream mem_stream;
    mem_stream_init(&mem_stream);

    struct log log = {
        .file = mem_stream.file,
        .disable_colors = true,
        .max_errors = 3,
        .source_name = "stdin",
        .source_data = STR_VIEW("abcd\nefgh\n")
    };
    struct source_pos begin = { .row = 1, .col = 1 };
    struct source_pos mid1  = { .row = 1, .col = 5, .bytes = 5 };
    struct source_pos mid2  = { .row = 2, .col = 1, .bytes = 6 };
    struct source_pos end   = { .row = 2, .col = 5, .bytes = log.source_data.length };
    log_error(&log, &(struct source_range) { .begin = begin, .end = mid1 }, "%d", 1);
    log_error(&log, &(struct source_range) { .begin = mid2,  .end = end },  "%d", 2);
    log_warn(&log, &(struct source_range) { .begin = begin, .end = end },  "%d", 3);
    log_note(&log, NULL, "%d", 4);
    log_error(&log, &(struct source_range) { .begin = begin, .end = end },  "%d", 5);
    mem_stream_destroy(&mem_stream);
    static const char* result =
        "error: 1\n"
        "  in stdin(1:1 - 1:5)\n"
        "   |\n"
        " 1 |abcd\n"
        "   |^^^^\n"
        "\n"
        "error: 2\n"
        "  in stdin(2:1 - 2:5)\n"
        "   |\n"
        " 2 |efgh\n"
        "   |^^^^\n"
        "\n"
        "warning: 3\n"
        "  in stdin(1:1 - 2:5)\n"
        "   |\n"
        " 1 |abcd\n"
        "   |^...\n"
        "note: 4\n";
    REQUIRE(strcmp(mem_stream.buf, result) == 0);
    free(mem_stream.buf);
}
