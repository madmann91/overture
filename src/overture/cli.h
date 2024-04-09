#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/// @file
///
/// Command-line interface parser. This module simplifies command-line argument parsing by providing
/// data types and functions to declaratively produce a command-line interface parser. Here is an
/// example of a parser built using this module:
///
/// ```c
/// // in main()
/// struct cli_option cli_options[] = {
///     { .short_name = "-h", .long_name = "--help", .parse = usage },
///     { .long_name = "--version", .parse = version },
///     cli_option_string(NULL, "--codegen", &options.codegen),
///     cli_flag(NULL, "--no-color",   &options.disable_colors),
///     cli_flag(NULL, "--no-cleanup", &options.disable_cleanup),
///     cli_flag("-v", "--verbose",    &options.is_verbose)
/// };
/// if (!cli_parse_options(argc, argv, cli_options, sizeof(cli_options) / sizeof(cli_options[0])))
///     return 1;
/// ```

/// Command-line parser state.
enum cli_state {
    CLI_STATE_ACCEPTED,
    CLI_STATE_REJECTED,
    CLI_STATE_ERROR
};

/// Command-line interface option.
struct cli_option {
    const char* short_name;         /// Short name for the option (with prefix `-`).
    const char* long_name;          /// Long name for the option (with prefix `--`).
    bool has_value;                 /// Whether this option expects an argument or not.
    void* data;                     /// User data passed to `parse()`.

    /// Option parsing callback, which takes the user data and the command-line argument as a
    /// string, and returns `true` on success, `false` otherwise.
    enum cli_state (*parse)(void*, char*);
};

/// @cond PRIVATE
static inline enum cli_state cli_set_flag(void* data, char*) {
    return *(bool*)data = true, CLI_STATE_ACCEPTED;
}

static inline enum cli_state cli_set_uint32(void* data, char* arg) {
    return *(uint32_t*)data = strtoul(arg, NULL, 10), CLI_STATE_ACCEPTED;
}

static inline enum cli_state cli_set_uint64(void* data, char* arg) {
    return *(uint64_t*)data = strtoumax(arg, NULL, 10), CLI_STATE_ACCEPTED;
}

static inline enum cli_state cli_set_string(void* data, char* arg) {
    return *(char**)data = arg, CLI_STATE_ACCEPTED;
}
/// @endcond

/// Produces a boolean flag with no arguments.
[[nodiscard]] static inline struct cli_option cli_flag(
    const char* short_name,
    const char* long_name,
    bool* data)
{
    return (struct cli_option) {
        .short_name = short_name,
        .long_name = long_name,
        .data = data,
        .parse = cli_set_flag
    };
}

/// Produces an option that takes a 32-bit unsigned integer as an argument.
[[nodiscard]] static inline struct cli_option cli_option_uint32(
    const char* short_name,
    const char* long_name,
    uint32_t* data)
{
    return (struct cli_option) {
        .short_name = short_name,
        .long_name = long_name,
        .data = data,
        .parse = cli_set_uint32,
        .has_value = true
    };
}

/// Produces an option that takes a 64-bit unsigned integer as an argument.
[[nodiscard]] static inline struct cli_option cli_option_uint64(
    const char* short_name,
    const char* long_name,
    uint64_t* data)
{
    return (struct cli_option) {
        .short_name = short_name,
        .long_name = long_name,
        .data = data,
        .parse = cli_set_uint64,
        .has_value = true
    };
}

/// Produces an option that takes a string as an argument.
[[nodiscard]] static inline struct cli_option cli_option_string(
    const char* short_name,
    const char* long_name,
    char** data)
{
    return (struct cli_option) {
        .short_name = short_name,
        .long_name = long_name,
        .data = data,
        .parse = cli_set_string,
        .has_value = true
    };
}

/// Sets an argument to `NULL` and return the old value.
static inline char* cli_take_arg(char** argv, int i) {
    char* arg = argv[i];
    argv[i] = NULL;
    return arg;
}

/// Tries to match a given command-line option against an argument.
/// @param argc The value of `argc`, as passed to `main()`.
/// @param argv The value of `argv`, as passed to `main()`.
/// @param i Pointer to the current argument index.
/// @param option The option to match against the command line argument `argv[*i]`.
/// @return A state indicating whether the argument was matched or not.
static inline enum cli_state cli_accept_option(
    int argc, char** argv, int* i,
    const struct cli_option* option)
{
    if (option->short_name && !strcmp(argv[*i], option->short_name)) {
        if (!option->has_value) {
            cli_take_arg(argv, *i);
            return option->parse(option->data, NULL);
        }
        if (*i + 1 >= argc)
            goto missing_argument;
        cli_take_arg(argv, *i);
        return option->parse(option->data, cli_take_arg(argv, ++(*i)));
    } else if (option->long_name) {
        size_t name_len = strlen(option->long_name);
        if (strncmp(argv[*i], option->long_name, name_len))
            return CLI_STATE_REJECTED;
        if (!option->has_value) {
            if (argv[*i][name_len] != 0)
                return CLI_STATE_REJECTED;
            cli_take_arg(argv, *i);
            return option->parse(option->data, NULL);
        }
        if (argv[*i][name_len] == '=') {
            return option->parse(option->data, cli_take_arg(argv, *i) + name_len + 1);
        } else if (argv[*i][name_len] == 0) {
            if (*i + 1 >= argc)
                goto missing_argument;
            cli_take_arg(argv, *i);
            return option->parse(option->data, cli_take_arg(argv, ++(*i)));
        }
    }
    return CLI_STATE_REJECTED;

missing_argument:
    fprintf(stderr, "missing argument for '%s'\n", argv[*i]);
    return CLI_STATE_ERROR;
}

/// Parses the standard argument `argc`, and `argv` using the given array of options.
static inline bool cli_parse_options(
    int argc, char** argv,
    const struct cli_option* options,
    size_t option_count)
{
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-')
            continue;
        for (size_t j = 0; j < option_count; ++j) {
            enum cli_state state = cli_accept_option(argc, argv, &i, &options[j]);
            if (state == CLI_STATE_ACCEPTED)
                goto next_arg;
            if (state == CLI_STATE_ERROR)
                return false;
        }
        fprintf(stderr, "invalid option '%s'\n", argv[i]);
        return false;
next_arg:
        continue;
    }
    return true;
}
