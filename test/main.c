#include <overture/test.h>
#include <overture/term.h>
#include <overture/cli.h>

#include <stdio.h>

struct options {
    bool disable_colors;
};

static enum cli_state usage(void*, char*) {
    printf(
        "usage: testdriver [options] filters ...\n"
        "options:\n"
        "   -h    --help       Shows this message.\n"
        "         --no-color   Turns of the use of color in the output.\n"
        "         --list       Lists all tests and exit.\n");
    return CLI_STATE_ERROR;
}

static enum cli_state print_tests(void*, char*) {
    print_test_names(stdout);
    return CLI_STATE_ERROR;
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

    filter_tests(argc, argv);
    return run_tests(options.disable_colors) ? 0 : 1;
}
