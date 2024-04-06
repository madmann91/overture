#pragma once

#include "str.h"

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

/// @file
///
/// Error or warning log information. This can be used to produce accurate error messages for
/// compilers or parsers.

/// Message type.
enum msg_tag {
    MSG_ERR,    ///< Error message.
    MSG_WARN,   ///< Warning message.
    MSG_NOTE    ///< Note attached to either a warning or error message.
};

/// Position in a source file.
struct source_pos {
    uint32_t row;   ///< Source file row (1-based).
    uint32_t col;   ///< Source file column (1-based).
    size_t bytes;   ///< Number of bytes away from the beginning of the file.
};

/// Range of characters in a source file.
struct source_range {
    struct source_pos begin, end;
};

/// Log associated to a single source file.
struct log {
    FILE* file;                     ///< Stream where messages are shown.
    bool disable_colors;            ///< Flag controlling whether colors are enabled or not.
    size_t max_errors;              ///< Maximum number of errors before the log stops displaying them.
    size_t error_count;             ///< Current number of errors.
    const char* source_name;        ///< Source file name.
    struct str_view source_data;    ///< Source file data (for diagnostics).
};

/// Prints a log message.
/// @param msg_tag Type of message to show.
/// @param log The log where the message is printed.
/// @param source_range Range of characters in the source file which the message refers to (may be `NULL`).
/// @param fmt Formatting string, following the syntax of `printf`.
/// @param args Argument list.
/// If `source_range` is NULL, no diagnostic is printed, and the source file name is not displayed
/// in the output.
void log_msg(
    enum msg_tag msg_tag,
    struct log* log,
    const struct source_range* source_range,
    const char* fmt,
    va_list args);

/// Prints an error message.
/// @see log_msg.
[[gnu::format(printf, 3, 4)]]
void log_error(struct log*, const struct source_range*, const char* fmt, ...);

/// Prints an warning message.
/// @see log_msg.
[[gnu::format(printf, 3, 4)]]
void log_warn(struct log*, const struct source_range*, const char* fmt, ...);

/// Prints a note.
/// @see log_msg.
[[gnu::format(printf, 3, 4)]]
void log_note(struct log*, const struct source_range*, const char* fmt, ...);
