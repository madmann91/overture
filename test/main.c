#include <overture/test_macros.h>
#include <overture/test.h>
#include <overture/term.h>
#include <overture/cli.h>

#include <stdio.h>

struct options {
    bool disable_colors;
};

static bool usage(void*, char*) {
    printf(
        "usage: testdriver [options] filters ...\n"
        "options:\n"
        "   -h    --help       Shows this message.\n"
        "         --no-color   Turns of the use of color in the output.\n"
        "         --list       Lists all tests and exit.\n");
    return false;
}

static bool print_tests(void*, char*) {
    VEC_FOREACH(struct test, test, tests) {
        printf("%s\n", test->name);
    }
    return false;
}

static const char* color_code(bool success) {
    return success
        ? TERM2(TERM_FG_GREEN, TERM_BOLD)
        : TERM2(TERM_FG_RED, TERM_BOLD);
}

static size_t find_longest_test_name(void) {
    size_t max_width = 0;
    VEC_FOREACH(struct test, test, tests) {
        if (!test->enabled)
            continue;
        size_t width = strlen(test->name);
        max_width = max_width < width ? width : max_width;
    }
    return max_width;
}

static size_t filter_tests(int argc, char** argv, const struct options*) {
    bool enable_filtering = false;
    for (int i = 1; i < argc && !enable_filtering; ++i)
        enable_filtering |= argv[i] != NULL;
    if (!enable_filtering) {
        VEC_FOREACH(struct test, test, tests) { test->enabled = true; }
        return tests.elem_count;
    }

    for (int i = 1; i < argc; ++i) {
        if (!argv[i])
            continue;
        VEC_FOREACH(struct test, test, tests) {
            test->enabled |= !strcmp(test->name, argv[i]);
        }
    }

    size_t enabled_tests = 0;
    VEC_FOREACH(struct test, test, tests) {
        enabled_tests += test->enabled ? 1 : 0;
    }
    return enabled_tests;
}

int main(int argc, char** argv) {
    struct options options = { .disable_colors = !is_term(stdout) };
    struct cli_option cli_options[] = {
        { .short_name = "-h", .long_name = "--help", .parse = usage },
        { .long_name = "--list", .parse = print_tests },
        cli_flag(NULL, "--no-color", &options.disable_colors),
    };
    if (!cli_parse_options(argc, argv, cli_options, sizeof(cli_options) / sizeof(cli_options[0])))
        return 1;

    size_t enabled_tests = filter_tests(argc, argv, &options);
    if (enabled_tests == 0) {
        fprintf(stderr, "no tests match filters\n");
        return 1;
    }

    printf("running %zu test(s):\n\n", enabled_tests);

    run_tests();

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
            options.disable_colors ? "" : color_code(test->status == TEST_PASSED), msg,
            options.disable_colors ? "" : TERM1(TERM_RESET));
        passed_asserts += test->passed_asserts;
        passed_tests += test->status == TEST_PASSED ? 1 : 0;
    }

    bool failed = passed_tests != enabled_tests;
    printf("\n%s%zu/%zu test(s) passed, %zu assertion(s) passed%s\n",
        options.disable_colors ? "" : color_code(failed == 0),
        passed_tests, enabled_tests, passed_asserts,
        options.disable_colors ? "" : TERM1(TERM_RESET));
    test_vec_destroy(&tests);
    return failed == 0 ? 0 : 1;
}
