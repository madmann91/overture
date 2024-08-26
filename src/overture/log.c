#include "log.h"
#include "mem.h"
#include "term.h"

#include <stdarg.h>
#include <stdlib.h>
#include <inttypes.h>

struct styles {
    const char* msg;
    const char* range;
    const char* reset;
};

struct line_size log_print_line(struct log* log, const struct file_loc* loc) {
    struct line_size line_size = {};
    FILE* file = fopen(loc->file_name, "rb");
    if (!file)
        return line_size;

    fseek(file, loc->begin.bytes + 1 - loc->begin.col, SEEK_SET);
    for (size_t col = 1;; ++col) {
        int c = fgetc(file);
        if (c == EOF || c == '\n')
            break;
        size_t char_count = 0;
        if (c == '\t')
            fputs("    ", log->file), char_count = 4;
        else
            fputc(c, log->file), char_count = 1;
        bool is_left   = col < loc->begin.col;
        bool is_inside = !is_left && (loc->begin.row < loc->end.row || col < loc->end.col);
        line_size.left   += is_left   ? char_count : 0;
        line_size.inside += is_inside ? char_count : 0;
    }

    fclose(file);
    return line_size;
}

static inline int count_digits(uint32_t i) {
    int count = 1;
    while (i >= 10)
        count++, i /= 10;
    return count;
}

static inline void print_diagnostic(
    struct log* log,
    const struct file_loc* loc,
    const struct styles* styles)
{
    int indent_size = count_digits(loc->end.row);

    fprintf(log->file, " %s%*s%s ", styles->range, indent_size, " ", styles->reset);
    fprintf(log->file, "%s|%s\n", styles->msg, styles->reset);

    fprintf(log->file, " %s%*"PRIu32"%s ", styles->range, indent_size, loc->begin.row, styles->reset);
    fprintf(log->file, "%s|%s", styles->msg, styles->reset);
    struct line_size line_size = log->print_line(log, loc);
    fputs("\n", log->file);

    fprintf(log->file, " %s%*s%s ", styles->range, indent_size, " ", styles->reset);
    fprintf(log->file, "%s|%s%*s", styles->msg, styles->reset, (int)line_size.left, "");
    fputs(styles->msg, log->file);
    if (loc->begin.row == loc->end.row) {
        for (size_t i = 0; i < line_size.inside; ++i)
            fputc('^', log->file);
    } else {
        fputc('^', log->file);
        for (size_t i = 1; i < line_size.inside; ++i)
            fputc('.', log->file);
    }
    fputs(styles->reset, log->file);
    fputc('\n', log->file);
}

void log_msg(
    enum msg_tag tag,
    struct log* log,
    const struct file_loc* loc,
    const char* fmt,
    va_list args)
{
    if (log->error_count >= log->max_errors || log->warn_count >= log->max_warns)
        return;

    log->error_count += tag == MSG_ERR ? 1 : 0;
    log->warn_count  += tag == MSG_WARN ? 1 : 0;

    if (!log->file)
        return;

    if (tag != MSG_NOTE && (log->error_count + log->warn_count) > 1)
        fprintf(log->file, "\n");

    static const char* msg_styles[] = {
        [MSG_ERR ] = TERM2(TERM_FG_RED, TERM_BOLD),
        [MSG_WARN] = TERM2(TERM_FG_YELLOW, TERM_BOLD),
        [MSG_NOTE] = TERM2(TERM_FG_CYAN, TERM_BOLD)
    };
    static const char* msg_header[] = {
        [MSG_ERR]  = "error",
        [MSG_WARN] = "warning",
        [MSG_NOTE] = "note"
    };

    struct styles styles = {
        .msg   = log->disable_colors ? "" : msg_styles[tag],
        .range = log->disable_colors ? "" : TERM2(TERM_FG_WHITE, TERM_BOLD),
        .reset = log->disable_colors ? "" : TERM1(TERM_RESET)
    };

    fprintf(log->file, "%s%s%s: ", styles.msg, msg_header[tag], styles.reset);
    vfprintf(log->file, fmt, args);
    fprintf(log->file, "\n");

    if (loc && loc->file_name) {
        fprintf(log->file, "  in ");
        fprintf(log->file, "%s%s(%"PRIu32":%"PRIu32" - %"PRIu32":%"PRIu32")%s\n",
            styles.range,
            loc->file_name,
            loc->begin.row,
            loc->begin.col,
            loc->end.row,
            loc->end.col,
            styles.reset);

        if (log->print_line)
            print_diagnostic(log, loc, &styles);
    }
}

void log_error(struct log* log, const struct file_loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg(MSG_ERR, log, loc, fmt, args);
    va_end(args);
}

void log_warn(struct log* log, const struct file_loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg(MSG_WARN, log, loc, fmt, args);
    va_end(args);
}

void log_note(struct log* log, const struct file_loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg(MSG_NOTE, log, loc, fmt, args);
    va_end(args);
}
