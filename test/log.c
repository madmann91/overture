#include <overture/test.h>
#include <overture/mem_stream.h>
#include <overture/log.h>

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

static struct str_view read_line(
    [[maybe_unused]] void* data,
    [[maybe_unused]] struct str_view file_name,
    uint32_t line)
{
    static const char* lines[] = {
        "    ab    cd",
        " efgh"
    };
    if (line <= 0 || line >= 3)
        return (struct str_view) {};
    return STR_VIEW(lines[line - 1]);
}

TEST(log) {
    struct mem_stream mem_stream;
    mem_stream_init(&mem_stream);

    struct line_reader line_reader = { .read_line = read_line };
    struct log log = {
        .file = mem_stream.file,
        .disable_colors = true,
        .max_warns = 1,
        .max_errors = 2,
        .line_reader = &line_reader
    };

    struct str_view file_name = STR_VIEW("stdin");
    struct source_pos begin = { .row = 1, .col = 5 };
    struct source_pos mid1  = { .row = 1, .col = 13 };
    struct source_pos mid2  = { .row = 2, .col = 2 };
    struct source_pos end   = { .row = 2, .col = 6 };

    log_error(&log, &(struct file_loc) { .file_name = file_name, .begin = begin, .end = mid1 }, "%d", 1);
    log_error(&log, &(struct file_loc) { .file_name = file_name, .begin = mid2,  .end = end },  "%d", 2);
    log_warn(&log, &(struct file_loc) { .file_name = file_name, .begin = begin, .end = end },  "%d", 3);
    log_note(&log, NULL, "%d", 4);
    log_error(&log, &(struct file_loc) { .file_name = file_name, .begin = begin, .end = end },  "%d", 5);
    log_note(&log, NULL, "%d", 6);

    mem_stream_destroy(&mem_stream);

    REQUIRE(log.error_count == 3);
    REQUIRE(log.warn_count == 1);

    static const char* result =
        "error: 1\n"
        "  in stdin(1:5 - 1:13)\n"
        "   |\n"
        " 1 |    ab    cd\n"
        "   |    ^^^^^^^^\n"
        "\n"
        "error: 2\n"
        "  in stdin(2:2 - 2:6)\n"
        "   |\n"
        " 2 | efgh\n"
        "   | ^^^^\n"
        "\n"
        "warning: 3\n"
        "  in stdin(1:5 - 2:6)\n"
        "   |\n"
        " 1 |    ab    cd\n"
        "   |    ^.......\n"
        "note: 4\n";

    REQUIRE(strcmp(mem_stream.buf, result) == 0);
    free(mem_stream.buf);
}
