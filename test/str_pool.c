#include <overture/test.h>
#include <overture/str_pool.h>
#include <overture/mem_pool.h>

#include <stdio.h>

TEST(str_pool) {
    struct mem_pool mem_pool = mem_pool_create();
    struct str_pool* str_pool = str_pool_create(&mem_pool);
    REQUIRE(str_pool_insert(str_pool, "foo") == str_pool_insert(str_pool, "foo"));
    REQUIRE(str_pool_insert(str_pool, "bar") != str_pool_insert(str_pool, "foo"));
    const char* numbers[10] = {
        str_pool_insert(str_pool, "0"),
        str_pool_insert(str_pool, "1"),
        str_pool_insert(str_pool, "2"),
        str_pool_insert(str_pool, "3"),
        str_pool_insert(str_pool, "4"),
        str_pool_insert(str_pool, "5"),
        str_pool_insert(str_pool, "6"),
        str_pool_insert(str_pool, "7"),
        str_pool_insert(str_pool, "8"),
        str_pool_insert(str_pool, "9"),
    };
    for (int i = 0; i < 10; ++i) {
        char buf[2];
        snprintf(buf, 2, "%d", i);
        REQUIRE(str_pool_insert_view(str_pool, (struct str_view) { .data = buf, .length = 1 }) == numbers[i]);
    }
    str_pool_destroy(str_pool);
    mem_pool_destroy(&mem_pool);
}
