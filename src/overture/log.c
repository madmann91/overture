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

    struct str_view line = log->line_reader->read_line(log->line_reader->data, loc->file_name, loc->begin.row);
    fprintf(log->file, " %s%*"PRIu32"%s ", styles->range, indent_size, loc->begin.row, styles->reset);
    fprintf(log->file, "%s|%s%.*s\n", styles->msg, styles->reset, (int)line.length, line.data);

    fprintf(log->file, " %s%*s%s ", styles->range, indent_size, " ", styles->reset);
    fprintf(log->file, "%s|%s%*s", styles->msg, styles->reset, (int)loc->begin.col - 1, "");
    fputs(styles->msg, log->file);
    if (loc->begin.row == loc->end.row) {
        for (size_t i = loc->begin.col; i < loc->end.col; ++i)
            fputc('^', log->file);
    } else {
        fputc('^', log->file);
        for (size_t i = loc->begin.col; i < line.length; ++i)
            fputc('.', log->file);
    }
    fputs(styles->reset, log->file);
    fputc('\n', log->file);
}

void log_msg_from_args(
    enum msg_tag tag,
    struct log* log,
    const struct file_loc* loc,
    const char* fmt,
    va_list args)
{
    if (log->warns_as_errors && tag == MSG_WARN)
        tag = MSG_ERROR;

    log->error_count += tag == MSG_ERROR ? 1 : 0;
    log->warn_count  += tag == MSG_WARN ? 1 : 0;

    if (log->error_count > log->max_errors || log->warn_count > log->max_warns)
        return;

    if (!log->file)
        return;

    if (tag != MSG_NOTE && (log->error_count + log->warn_count) > 1)
        fprintf(log->file, "\n");

    static const char* msg_styles[] = {
        [MSG_ERROR] = TERM2(TERM_FG_RED, TERM_BOLD),
        [MSG_WARN ] = TERM2(TERM_FG_YELLOW, TERM_BOLD),
        [MSG_NOTE ] = TERM2(TERM_FG_CYAN, TERM_BOLD)
    };

    static const char* msg_header[] = {
        [MSG_ERROR] = "error",
        [MSG_WARN ] = "warning",
        [MSG_NOTE ] = "note"
    };

    struct styles styles = {
        .msg   = log->disable_colors ? "" : msg_styles[tag],
        .range = log->disable_colors ? "" : TERM2(TERM_FG_WHITE, TERM_BOLD),
        .reset = log->disable_colors ? "" : TERM1(TERM_RESET)
    };

    fprintf(log->file, "%s%s%s: ", styles.msg, msg_header[tag], styles.reset);
    vfprintf(log->file, fmt, args);
    fprintf(log->file, "\n");

    if (loc) {
        fprintf(log->file, "  in %s%.*s(%"PRIu32":%"PRIu32" - %"PRIu32":%"PRIu32")%s\n",
            styles.range,
            (int)loc->file_name.length,
            loc->file_name.data,
            loc->begin.row,
            loc->begin.col,
            loc->end.row,
            loc->end.col,
            styles.reset);

        if (log->line_reader)
            print_diagnostic(log, loc, &styles);
    }
}

void log_msg(enum msg_tag tag, struct log* log, const struct file_loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg_from_args(tag, log, loc, fmt, args);
    va_end(args);
}

void log_error(struct log* log, const struct file_loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg_from_args(MSG_ERROR, log, loc, fmt, args);
    va_end(args);
}

void log_warn(struct log* log, const struct file_loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg_from_args(MSG_WARN, log, loc, fmt, args);
    va_end(args);
}

void log_note(struct log* log, const struct file_loc* loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_msg_from_args(MSG_NOTE, log, loc, fmt, args);
    va_end(args);
}
