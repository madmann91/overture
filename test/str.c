#include <overture/test_macros.h>
#include <overture/str.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TEST(str) {
    struct str s = str_create();
    str_printf(&s, "%s ", "Hello");
    str_printf(&s, "%s!", "world");
    REQUIRE(strcmp(str_terminate(&s), "Hello world!") == 0);
    str_destroy(&s);
}
