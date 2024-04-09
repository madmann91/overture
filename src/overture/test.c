#include "test.h"
#include "term.h"

#include <stdio.h>

#ifdef TEST_DISABLE_FORK
#include <setjmp.h>
#else
#include <sys/types.h>
#endif

enum test_status {
    TEST_NOT_RUN,
    TEST_PASSED,
    TEST_FAILED,
    TEST_SEGFAULT
};

struct test {
    const char* name;
    bool enabled;
    enum test_status status;
    size_t passed_asserts;

    void (*test_func) (struct test_context*);

#ifndef TEST_DISABLE_FORK
    pid_t pid;
    int read_pipe;
#endif
};

struct test_context {
#ifndef TEST_DISABLE_FORK
    int write_pipe;
#else
    jmp_buf buf;
#endif
    struct test* test;
    size_t passed_asserts;
};

VEC_DEFINE(test_vec, struct test, PRIVATE)

static struct test_vec tests;

static inline size_t count_enabled_tests(void) {
    size_t count = 0;
    VEC_FOREACH(struct test, test, tests)
        count += test->enabled ? 1 : 0;
    return count;
}

static bool check_tests_enabled(void) {
    size_t enabled_count = count_enabled_tests();
    if (enabled_count == 0) {
        fprintf(stderr, "no tests are enabled\n");
        return false;
    }

    printf("running %zu test(s):\n\n", enabled_count);
    return true;
}

static inline size_t find_longest_test_name(void) {
    size_t max_width = 0;
    VEC_FOREACH(struct test, test, tests) {
        if (!test->enabled)
            continue;
        size_t width = strlen(test->name);
        max_width = max_width < width ? width : max_width;
    }
    return max_width;
}

static const char* color_code(bool success) {
    return success
        ? TERM2(TERM_FG_GREEN, TERM_BOLD)
        : TERM2(TERM_FG_RED, TERM_BOLD);
}

static bool summarize_tests(bool disable_colors) {
    size_t max_width = find_longest_test_name();
    size_t passed_tests = 0;
    size_t passed_asserts = 0;
    VEC_FOREACH(struct test, test, tests) {
        if (!test->enabled)
            continue;

        const char* msg =
            test->status == TEST_PASSED ? "[PASSED]" :
            test->status == TEST_FAILED ? "[FAILED]" :
            test->status == TEST_SEGFAULT ? "[SEGFAULT]" :
            "[UNKNOWN]";
        printf(" %*s .............................. %s%s%s\n",
            (int)max_width, test->name,
            disable_colors ? "" : color_code(test->status == TEST_PASSED), msg,
            disable_colors ? "" : TERM1(TERM_RESET));
        passed_asserts += test->passed_asserts;
        passed_tests += test->status == TEST_PASSED ? 1 : 0;
    }

    size_t enabled_tests = count_enabled_tests();
    bool failed = passed_tests != enabled_tests;
    printf("\n%s%zu/%zu test(s) passed, %zu assertion(s) passed%s\n",
        disable_colors ? "" : color_code(failed == 0),
        passed_tests, enabled_tests, passed_asserts,
        disable_colors ? "" : TERM1(TERM_RESET));
    return !failed;
}

[[gnu::destructor]]
static void cleanup_tests(void) {
    test_vec_destroy(&tests);
    memset(&tests, 0, sizeof(struct test_vec));
}

static inline void print_failed_assert(
    struct test_context* context,
    const char* msg,
    const char* file,
    unsigned line)
{
    fprintf(stderr, "[%s] Assertion '%s' failed (%s:%u)\n", context->test->name, msg, file, line);
}

void print_tests(FILE* file) {
    VEC_FOREACH(struct test, test, tests)
        fprintf(file, "%s\n", test->name);
}

void filter_tests(int argc, char** argv) {
    bool did_match = false;
    for (int i = 1; i < argc; ++i) {
        if (!argv[i])
            continue;
        did_match = true;
        size_t len = strlen(argv[i]);
        VEC_FOREACH(struct test, test, tests)
            test->enabled |= strncmp(test->name, argv[i], len) == 0;
    }
    if (!did_match) {
        VEC_FOREACH(struct test, test, tests)
            test->enabled = true;
    }
}

void require_success(struct test_context* context) {
    context->passed_asserts++;
}

void register_test(const char* name, void (*test_func) (struct test_context*)) {
    test_vec_push(&tests, &(struct test) {
        .name = name,
        .status = TEST_NOT_RUN,
        .test_func = test_func
    });
}

#ifndef TEST_DISABLE_FORK
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void require_fail(
    struct test_context* context,
    const char* msg,
    const char* file,
    unsigned line)
{
    write(context->write_pipe, context, sizeof(struct test_context));
    close(context->write_pipe);
    print_failed_assert(context, msg, file, line);
    abort();
}

static inline struct test* find_test_by_pid(pid_t pid) {
    VEC_FOREACH(struct test, test, tests) {
        if (test->pid == pid)
            return test;
    }
    return NULL;
}

static void start_test(struct test* test) {
    int pipes[2];
    pipe(pipes);

    pid_t pid = fork();
    if (pid == 0) {
        close(pipes[0]);
        struct test_context context = {
            .test = test,
            .write_pipe = pipes[1]
        };
        test->test_func(&context);
        write(context.write_pipe, &context, sizeof(struct test_context));
        close(context.write_pipe);
        test_vec_destroy(&tests);
        exit(0);
    }

    close(pipes[1]);
    test->pid = pid;
    test->read_pipe = pipes[0];
}

bool run_tests(bool disable_colors) {
    if (!check_tests_enabled())
        return false;

    // Flush output buffers in case the test case prints something (fork may cause the file buffer
    // to be copied to the child process, making the program print the same thing twice).
    fflush(stdout);
    fflush(stderr);

    size_t enabled_tests = 0;
    VEC_FOREACH(struct test, test, tests) {
        if (test->enabled) {
            start_test(test);
            enabled_tests++;
        }
    }

    for (size_t i = 0; i < enabled_tests; ++i) {
        int status;
        pid_t test_pid = wait(&status);

        struct test* test = find_test_by_pid(test_pid);
        if (!test)
            continue;

        test->status = TEST_FAILED;
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            test->status = TEST_PASSED;
        else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
            test->status = TEST_SEGFAULT;

        struct test_context context;
        read(test->read_pipe, &context, sizeof(struct test_context));
        close(test->read_pipe);

        test->passed_asserts += context.passed_asserts;
    }

    return summarize_tests(disable_colors);
}
#else
void require_fail(
    struct test_context* context,
    const char* msg,
    const char* file,
    unsigned line)
{
    print_failed_assert(context, msg, file, line);
    longjmp(context->buf, 1);
}

static void start_test(struct test* test) {
    struct test_context context = { .test = test };
    if (setjmp(context.buf) != 0) {
        test->status = TEST_FAILED;
        return;
    }
    test->test_func(&context);
    test->status = TEST_PASSED;
    test->passed_asserts = context.passed_asserts;
}

bool run_tests(bool disable_colors) {
    if (!check_tests_enabled())
        return false;
    VEC_FOREACH(struct test, test, tests) {
        if (test->enabled)
            start_test(test);
    }
    return summarize_tests(disable_colors);
}
#endif
