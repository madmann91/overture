#pragma once

#include <stdio.h>
#include <stdbool.h>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

/// @file
///
/// Macros to work with ANSI terminal codes.

/// Produce a terminal code sequence with only one element.
#define TERM1(x)       "\33[" x "m"
/// Produce a terminal code sequence with two elements.
#define TERM2(x, y)    "\33[" x ";" y "m"
/// Produce a terminal code sequence with three elements.
#define TERM3(x, y, z) "\33[" x ";" y ";" z "m"

/// Resets the color and style.
#define TERM_RESET "0"

/// @name Font styles
/// @{

#define TERM_BOLD "1"
#define TERM_ITALIC "3"
#define TERM_UNDERLINE "4"

/// @}

/// @name Foreground colors
/// @{

#define TERM_FG_BLACK   "30"
#define TERM_FG_RED     "31"
#define TERM_FG_GREEN   "32"
#define TERM_FG_YELLOW  "33"
#define TERM_FG_BLUE    "34"
#define TERM_FG_MAGENTA "35"
#define TERM_FG_CYAN    "36"
#define TERM_FG_WHITE   "37"

/// @}

/// @name Background colors
/// @{

#define TERM_BG_BLACK   "40"
#define TERM_BG_RED     "41"
#define TERM_BG_GREEN   "42"
#define TERM_BG_YELLOW  "43"
#define TERM_BG_BLUE    "44"
#define TERM_BG_MAGENTA "45"
#define TERM_BG_CYAN    "46"
#define TERM_BG_WHITE   "47"

/// @}

/// @return `true` if the given stream is a terminal, `false` otherwise.
/// This may be used as a way to detect when to turn on/off ANSI color codes in the output.
[[nodiscard]] static inline bool is_term(FILE* file) {
    return isatty(fileno(file));
}
