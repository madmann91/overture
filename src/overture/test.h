#pragma once

#include "vec.h"

#include <stddef.h>
#include <stdbool.h>

#ifndef TEST_DISABLE_FORK
#include <sys/types.h>
#endif

struct test_context;

/// Test status.
enum test_status {
    TEST_NOT_RUN,
    TEST_PASSED,
    TEST_FAILED,
    TEST_SEGFAULT
};

/// Data structure representing a test.
struct test {
    const char* name;        ///< Test name.
    bool enabled;            ///< Flag controlling whether this test should be enabled or not.
    enum test_status status; ///< Test status.
    size_t passed_asserts;   ///< Number of assertions that passed. Only valid after the test was run.

    /// Function representing the test. The context passed as argument is used to track the
    /// execution status.
    void (*test_func) (struct test_context*);

#ifndef TEST_DISABLE_FORK
    pid_t pid;              ///< Process ID of the process running the test.
    int read_pipe;          ///< Pipe to read the status from.
#endif
};

VEC_DECL(test_vec, struct test, PUBLIC)

extern struct test_vec tests;

/// Run all the tests that have been registered previously, and that are enabled.
void run_tests(void);
