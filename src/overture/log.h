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
    MSG_ERROR,  ///< Error message.
    MSG_WARN,   ///< Warning message.
    MSG_NOTE    ///< Note attached to either a warning or error message.
};

/// Position in a source file.
struct source_pos {
    uint32_t row;   ///< Source file row (1-based).
    uint32_t col;   ///< Source file column (1-based).
    size_t bytes;   ///< Number of bytes away from the beginning of the file.
};

/// Location within a source file.
struct file_loc {
    const char* file_name;
    struct source_pos begin, end;
};

/// Size, in characters, of a source file line corresponding to a source file location.
struct line_size { 
    size_t left;    ///< Number of characters before the beginning of the location.
    size_t inside;  ///< Number of characters inside the location, until the end of the line (whichever ends first).
};

/// User-facing application log containing error and warning messages.
struct log {
    FILE* file;          ///< Stream where messages are shown.
    bool disable_colors; ///< Flag controlling whether colors are enabled or not.
    size_t max_errors;   ///< Maximum number of errors before the log stops displaying them.
    size_t max_warns;    ///< Maximum number of warnings before the log stops displaying them.
    size_t error_count;  ///< Current number of errors.
    size_t warn_count;   ///< Current number of warnings.

    /// Callback to use when printing diagnostics on the starting row of a given source file
    /// location. Returns the number of characters written on the left of, and inside the first row
    /// of the file location. When `NULL`, diagnostics are turned off.
    struct line_size (*print_line)(struct log*, const struct file_loc* loc);
};

/// Default implementation for `print_line`. This implementation opens the file mentioned in the
/// source file location on disk, and prints a line from it.
struct line_size log_print_line(struct log* log, const struct file_loc* loc);

/// Prints a log message.
/// @param msg_tag Type of message to show.
/// @param log The log where the message is printed.
/// @param loc Source file location that the log message is referring to. May be `NULL`, in which
///   case no diagnostic is printed, and the source file name is not displayed in the output.
/// @param fmt Formatting string, following the syntax of `printf`.
/// @param args Argument list.
void log_msg_from_args(
    enum msg_tag msg_tag,
    struct log* log,
    const struct file_loc* loc,
    const char* fmt,
    va_list args);

/// Prints a log message.
/// @see log_msg_from_args.
[[gnu::format(printf, 4, 5)]]
void log_msg(
    enum msg_tag msg_tag,
    struct log* log,
    const struct file_loc* loc,
    const char* fmt, ...);

/// Prints an error message.
/// @see log_msg.
[[gnu::format(printf, 3, 4)]]
void log_error(struct log*, const struct file_loc*, const char* fmt, ...);

/// Prints a warning message.
/// @see log_msg.
[[gnu::format(printf, 3, 4)]]
void log_warn(struct log*, const struct file_loc*, const char* fmt, ...);

/// Prints a note.
/// @see log_msg.
[[gnu::format(printf, 3, 4)]]
void log_note(struct log*, const struct file_loc*, const char* fmt, ...);
