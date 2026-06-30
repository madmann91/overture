#include <overture/test.h>
#include <overture/mem_stream.h>
#include <overture/log.h>

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

static struct file_line read_line(
    [[maybe_unused]] void* data,
    [[maybe_unused]] const char* file_name,
    uint32_t line)
{
    static const char* lines[] = {
        "    ab    cd",
        " efgh"
    };
    if (line <= 0 || line >= 3)
        return (struct file_line) {};
    return (struct file_line) {
        .is_valid = true,
        .contents = STR_VIEW(lines[line - 1])
    };
}

TEST(log) {
    struct mem_stream mem_stream;
    mem_stream_init(&mem_stream);

    struct line_reader line_reader = { .read_line = read_line };
    struct log log = {
        .file = mem_stream.file,
        .disable_colors = true,
        .max_warns = 2,
        .max_errors = 2,
        .line_reader = &line_reader
    };

    const char* file_name = "stdin";
    struct source_pos begin = { .row = 1, .col = 5 };
    struct source_pos mid1  = { .row = 1, .col = 13 };
    struct source_pos mid2  = { .row = 2, .col = 2 };
    struct source_pos end   = { .row = 2, .col = 6 };

    log_error(&log, &(struct file_loc) { .displayed_file_name = file_name, .displayed_line = begin.row, .begin = begin, .end = mid1 }, "%d", 1);
    log_error(&log, &(struct file_loc) { .displayed_file_name = file_name, .displayed_line = mid2.row, .begin = mid2,  .end = end },  "%d", 2);
    log_warn(&log, &(struct file_loc) { .displayed_file_name = file_name, .displayed_line = begin.row, .begin = begin, .end = end },  "%d", 3);
    log_note(&log, NULL, "%d", 4);
    log_error(&log, &(struct file_loc) { .displayed_file_name = file_name, .displayed_line = begin.row, .begin = begin, .end = end },  "%d", 5);
    log_note(&log, NULL, "%d", 6);
    log_warn(&log, &(struct file_loc) { .displayed_file_name = "foo", .displayed_line = 3, .begin = begin, .end = end },  "%d", 7);
    log_note(&log, NULL, "%d", 8);

    char* buf = mem_stream_release(&mem_stream);

    REQUIRE(log.error_count == 3);
    REQUIRE(log.warn_count == 2);

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
        "note: 4\n"
        "\n"
        "warning: 7\n"
        "  in foo(3:5 - 4:6)\n"
        "   |\n"
        " 3 |    ab    cd\n"
        "   |    ^.......\n"
        "note: 8\n";

    REQUIRE(strcmp(buf, result) == 0);
    free(buf);
}
