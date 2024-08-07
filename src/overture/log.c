#include "log.h"
#include "mem.h"
#include "term.h"

#include <stdarg.h>
#include <stdlib.h>
#include <inttypes.h>

#define ERROR_STYLE TERM2(TERM_FG_RED, TERM_BOLD)
#define WARN_STYLE  TERM2(TERM_FG_YELLOW, TERM_BOLD)
#define NOTE_STYLE TERM2(TERM_FG_CYAN, TERM_BOLD)
#define RANGE_STYLE TERM2(TERM_FG_WHITE, TERM_BOLD)
#define WITH_STYLE(x, y) (log->disable_colors ? (y) : x y TERM1(TERM_RESET))

static inline int count_digits(uint32_t i) {
    int count = 1;
    while (i >= 10)
        count++, i /= 10;
    return count;
}

static inline struct str_view line_at(
    const struct str_view* source_data,
    const struct source_pos* pos)
{
    size_t begin = pos->bytes;
    while (begin > 0 && source_data->data[begin - 1] != '\n')
        begin--;
    size_t end = pos->bytes;
    while (end < source_data->length && source_data->data[end] != '\n')
        end++;
    return (struct str_view) { .data = source_data->data + begin, .length = end - begin };
}

static inline void print_diagnostic(
    enum msg_tag tag,
    struct log* log,
    const struct source_range* source_range)
{
    const char* style = "";
    const char* reset_style = "";
    if (!log->disable_colors) {
        reset_style = TERM1(TERM_RESET);
        switch (tag) {
            case MSG_ERR:  style = ERROR_STYLE; break;
            case MSG_WARN: style = WARN_STYLE;  break;
            case MSG_NOTE: style = NOTE_STYLE;  break;
        }
    }

    int indent_size = count_digits(source_range->end.row);

    fprintf(log->file, WITH_STYLE(RANGE_STYLE, " %*s "), indent_size, " ");
    fprintf(log->file, "%s|%s", style, reset_style);
    fprintf(log->file, "\n");

    fprintf(log->file, WITH_STYLE(RANGE_STYLE, " %*"PRIu32" "), indent_size, source_range->begin.row);
    fprintf(log->file, "%s|%s", style, reset_style);
    struct str_view begin_line = line_at(&log->source_data, &source_range->begin);
    fprintf(log->file, "%.*s", (int)begin_line.length, begin_line.data);
    fprintf(log->file, "\n");

    fprintf(log->file, WITH_STYLE(RANGE_STYLE, " %*s "), indent_size, " ");
    fprintf(log->file, "%s|%s", style, reset_style);
    fprintf(log->file, "%*s", (int)source_range->begin.col - 1, "");
    fputs(style, log->file);
    if (source_range->begin.row == source_range->end.row) {
        for (uint32_t i = source_range->begin.col; i < source_range->end.col; ++i)
            fprintf(log->file, "^");
    } else {
        fprintf(log->file, "^...");
    }
    fputs(reset_style, log->file);
    fprintf(log->file, "\n");
}

void log_msg(
    enum msg_tag tag,
    struct log* log,
    const struct source_range* source_range,
    const char* fmt,
    va_list args)
{
    if ((tag == MSG_ERR && log->error_count >= log->max_errors) ||
        (tag == MSG_WARN && log->warn_count >= log->max_warns))
        return;

    log->error_count += tag == MSG_ERR ? 1 : 0;
    log->warn_count  += tag == MSG_WARN ? 1 : 0;

    if (!log->file)
        return;

    if (tag != MSG_NOTE && (log->error_count + log->warn_count) > 1)
        fprintf(log->file, "\n");

    switch (tag) {
        case MSG_ERR:  fprintf(log->file, WITH_STYLE(ERROR_STYLE, "error:"  )); break;
        case MSG_WARN: fprintf(log->file, WITH_STYLE(WARN_STYLE,  "warning:")); break;
        case MSG_NOTE: fprintf(log->file, WITH_STYLE(NOTE_STYLE,  "note:"   )); break;
    }

    fprintf(log->file, " ");
    vfprintf(log->file, fmt, args);
    fprintf(log->file, "\n");

    if (source_range && log->source_name) {
        fprintf(log->file, "  in ");
        fprintf(log->file, WITH_STYLE(RANGE_STYLE, "%s(%"PRIu32":%"PRIu32" - %"PRIu32":%"PRIu32")\n"),
            log->source_name,
            source_range->begin.row,
            source_range->begin.col,
            source_range->end.row,
            source_range->end.col);

        if (log->source_data.length > 0)
            print_diagnostic(tag, log, source_range);
    }
}

void log_error(struct log* log, const struct source_range* source_range, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg(MSG_ERR, log, source_range, fmt, args);
    va_end(args);
}

void log_warn(struct log* log, const struct source_range* source_range, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg(MSG_WARN, log, source_range, fmt, args);
    va_end(args);
}

void log_note(struct log* log, const struct source_range* source_range, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg(MSG_NOTE, log, source_range, fmt, args);
    va_end(args);
}
