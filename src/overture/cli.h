#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file 
 *
 * Command-line interface parser. This module simplifies command-line argument parsing by providing
 * data types and functions to declaratively produce a command-line interface parser. Here is an
 * example of a parser built using this module:
 *
 *      // in main()
 *      struct cli_option cli_options[] = {
 *          { .short_name = "-h", .long_name = "--help", .parse = usage },
 *          { .long_name = "--version", .parse = version },
 *          cli_option_string(NULL, "--codegen", &options.codegen),
 *          cli_flag(NULL, "--no-color",   &options.disable_colors),
 *          cli_flag(NULL, "--no-cleanup", &options.disable_cleanup),
 *          cli_flag("-v", "--verbose",    &options.is_verbose)
 *      };
 *      if (!cli_parse_options(argc, argv, cli_options, sizeof(cli_options) / sizeof(cli_options[0])))
 *          return 1;
 */

/// Command-line interface option.
struct cli_option {
    const char* short_name;         /// Short name for the option (with prefix `-`).
    const char* long_name;          /// Long name for the option (with prefix `--`).
    bool has_value;                 /// Whether this option expects an argument or not.
    void* data;                     /// User data passed to `parse()`.

    /// Option parsing callback, which takes the user data and the command-line argument as a
    /// string, and returns `true` on success, `false` otherwise.
    bool (*parse)(void*, char*);
};

/// Produces a boolean flag with no arguments.
[[nodiscard]] struct cli_option cli_flag(const char* short_name, const char* long_name, bool* result);
/// Produces an option that takes a 32-bit unsigned integer as an argument. 
[[nodiscard]] struct cli_option cli_option_uint32(const char* short_name, const char* long_name, uint32_t* arg);
/// Produces an option that takes a 64-bit unsigned integer as an argument. 
[[nodiscard]] struct cli_option cli_option_uint64(const char* short_name, const char* long_name, uint64_t* arg);
/// Produces an option that takes a string as an argument. 
[[nodiscard]] struct cli_option cli_option_string(const char* short_name, const char* long_name, char** arg);

/// Parses the standard argument `argc`, and `argv` using the given array of options.
bool cli_parse_options(
    int argc, char** argv,
    const struct cli_option*,
    size_t option_count);
