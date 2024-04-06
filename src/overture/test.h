#pragma once

#include "vec.h"

/// @file
///
/// Test framework for C programs. Supports process isolation on POSIX platforms. With this
/// framework, tests can be written with very little code:
///
/// ```c
/// TEST(my_test) {
///     REQUIRE(1 != 2);
/// }
/// ```
///
/// The snippet above creates a test named `my_test` that is automatically registered at startup.
/// A test driver can choose to enable or disable tests, and can then run the tests simply by
/// calling @ref run_tests:
///
/// ```c
/// reset_tests(false);
/// enable_matching_tests("foo");
/// run_tests();
/// ```
///
/// The code above automatically takes care of spawning processes and running every test.

#include <stddef.h>
#include <stdbool.h>

/// Declares a test with the given name. The test is automatically registered at startup. The test
/// is disabled by default, use @ref filter_test to enable it based on filters.
#define TEST(name) \
    void test_##name(struct test_context*); \
    __attribute__((constructor)) void register_##name() { register_test(#name, test_##name); } \
    void test_##name([[maybe_unused]] struct test_context* context)

/// Asserts that the given condition is `true`. Fails the test if it is not the case.
#define REQUIRE(x) \
    do { \
        if (!(x)) \
            require_fail(context, #x, __FILE__, __LINE__); \
        else \
            require_success(context); \
    } while (false)

struct test_context;

/// Run all the enabled tests, and then print the result on the standard output.
/// @return `true` on success, `false` otherwise.
bool run_tests(bool disable_colors);

/// Filters tests based on the names that appear in the given list of arguments.
/// If there are no filters, all tests are enabled. This means that running `filter_tests(0, NULL)`
/// enables all tests. Additionally, individual values in `argv` may be NULL.
/// @param argc Number of elements in @ref argv.
/// @param argv Prefixes to use for matching.
void filter_tests(int argc, char** argv);

/// Prints available test names, separated by new lines, on the given stream.
void print_test_names(FILE*);

/// Internal function called by the framework when a requirement fails. Do not call directly.
[[noreturn]] void require_fail(struct test_context*, const char*, const char*, unsigned);

/// Internal function called by the framework when a requirement succeeds. Do not call directly.
void require_success(struct test_context*);

/// Internal function to register a test. Do not call directly.
void register_test(const char*, void (*) (struct test_context*));
