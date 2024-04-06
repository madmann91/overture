#include <overture/test.h>
#include <overture/cli.h>

#include <string.h>

TEST(cli) {
    bool flag1 = false;
    bool flag2 = false;
    char* string_val = NULL;
    uint32_t u32_val = 0;
    uint64_t u64_val = 0;
    struct cli_option options[] = {
        cli_option_string(NULL, "--string-val", &string_val),
        cli_option_uint32(NULL, "--u32-val", &u32_val),
        cli_option_uint64("-u64", NULL, &u64_val),
        cli_flag("-f", NULL, &flag1),
        cli_flag(NULL, "--flag", &flag2)
    };
    size_t option_count = sizeof(options) / sizeof(options[0]);
    char* argv[] = {
        "exe-name",
        "-f",
        "--flag",
        "--string-val", "abcd",
        "--u32-val", "1234",
        "-u64", "5678",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    REQUIRE(cli_parse_options(argc, argv, options, option_count));
    REQUIRE(flag1);
    REQUIRE(flag2);
    REQUIRE(!strcmp(string_val, "abcd"));
    REQUIRE(u32_val == 1234);
    REQUIRE(u64_val == 5678);
}
