#include "test.h"
#include "test_macros.h"

#include <stdio.h>

VEC_IMPL(test_vec, struct test, PUBLIC)

struct test_vec tests = {};

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

void fail_test(
    struct test_context* context,
    const char* msg,
    const char* file,
    unsigned line)
{
    write(context->write_pipe, context, sizeof(struct test_context));
    close(context->write_pipe);
    fprintf(stderr, "Assertion '%s' failed (%s:%u)\n", msg, file, line);
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

void run_tests(void) {
    // Flush output buffers in case the test case prints something
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
}
#else
void fail_test(
    struct test_context* context,
    const char* msg,
    const char* file,
    unsigned line)
{
    fprintf(stderr, "Assertion '%s' failed (%s:%u)\n", msg, file, line);
    longjmp(context->buf, 1);
}

static void start_test(struct test* test) {
    struct test_context context = {};
    if (setjmp(context.buf) != 0) {
        test->status = TEST_FAILED;
        return;
    }
    test->test_func(&context);
    test->status = TEST_PASSED;
    test->passed_asserts = context.passed_asserts;
}

void run_tests(void) {
    VEC_FOREACH(struct test, test, tests) {
        if (test->enabled)
            start_test(test);
    }
}
#endif
