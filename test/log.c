#include <overture/test.h>
#include <overture/mem_stream.h>
#include <overture/log.h>

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

struct log_with_source {
    struct log log;
    const char* source_data;
};

static struct line_size print_line(struct log* log, const struct file_loc* loc) {
    const char* ptr = ((struct log_with_source*)log)->source_data + loc->begin.bytes + 1 - loc->begin.col;
    struct line_size line_size = {};
    for (size_t col = 1; *ptr != '\n' && *ptr != '\0'; ++col, ++ptr) {
        size_t char_count = 0;
        if (*ptr == '\t')
            fputs("    ", log->file), char_count = 4;
        else
            fputc(*ptr, log->file), char_count = 1;
        bool is_left   = col < loc->begin.col;
        bool is_inside = !is_left && (loc->begin.row < loc->end.row || col < loc->end.col);
        line_size.left   += is_left   ? char_count : 0;
        line_size.inside += is_inside ? char_count : 0;
    }
    return line_size;
}

TEST(log) {
    struct mem_stream mem_stream;
    mem_stream_init(&mem_stream);

    struct log_with_source log_with_source = {
        .log = {
            .file = mem_stream.file,
            .disable_colors = true,
            .max_warns = 1,
            .max_errors = 2,
            .print_line = print_line
        },
        .source_data = "\tab\tcd\n efgh\n"
    };

    struct log* log = &log_with_source.log;

    const char* file_name = "stdin";
    struct source_pos begin = { .row = 1, .col = 2, .bytes = 1 };
    struct source_pos mid1  = { .row = 1, .col = 7, .bytes = 6 };
    struct source_pos mid2  = { .row = 2, .col = 2, .bytes = 8 };
    struct source_pos end   = { .row = 2, .col = 6, .bytes = strlen(log_with_source.source_data) };

    log_error(log, &(struct file_loc) { .file_name = file_name, .begin = begin, .end = mid1 }, "%d", 1);
    log_error(log, &(struct file_loc) { .file_name = file_name, .begin = mid2,  .end = end },  "%d", 2);
    log_warn(log, &(struct file_loc) { .file_name = file_name, .begin = begin, .end = end },  "%d", 3);
    log_note(log, NULL, "%d", 4);
    log_error(log, &(struct file_loc) { .file_name = file_name, .begin = begin, .end = end },  "%d", 5);

    mem_stream_destroy(&mem_stream);

    static const char* result =
        "error: 1\n"
        "  in stdin(1:2 - 1:7)\n"
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
        "  in stdin(1:2 - 2:6)\n"
        "   |\n"
        " 1 |    ab    cd\n"
        "   |    ^.......\n"
        "note: 4\n";
    REQUIRE(strcmp(mem_stream.buf, result) == 0);
    free(mem_stream.buf);
}
