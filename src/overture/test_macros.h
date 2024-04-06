#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifdef TEST_DISABLE_FORK
#include <setjmp.h>
#endif

/// Context object passed to each test.
struct test_context {
#ifndef TEST_DISABLE_FORK
    int write_pipe;
#else
    jmp_buf buf;
#endif
    size_t passed_asserts;
};

/// Declares a test with the given name.
#define TEST(name) \
    void test_##name(struct test_context*); \
    __attribute__((constructor)) void register_##name() { register_test(#name, test_##name); } \
    void test_##name([[maybe_unused]] struct test_context* context)

/// Asserts that the given condition is `true`. Fails the test if it is not the case.
#define REQUIRE(x) \
    do { \
        if (!(x)) \
            fail_test(context, #x, __FILE__, __LINE__); \
        else \
            context->passed_asserts++; \
    } while (false)

[[noreturn]]
void fail_test(
    struct test_context*,
    const char* msg,
    const char* file,
    unsigned line);

void register_test(const char* name, void (*) (struct test_context*));
