#include <overture/test.h>
#include <overture/mem_pool.h>

#include <string.h>

struct foo {
    int x;
    double y;
    void* p;
};

TEST(mem_pool) {
    struct mem_pool mem_pool = mem_pool_create();
    char* str = MEM_POOL_ALLOC(mem_pool, char[9]);
    strcpy(str, "abcdabcd");
    double* d = MEM_POOL_ALLOC(mem_pool, double);
    *d = 34.0;
    struct foo* foo = MEM_POOL_ALLOC(mem_pool, struct foo);
    foo->x = 42;
    foo->y = *d;
    foo->p = NULL;
    REQUIRE(!strcmp(str, "abcdabcd"));
    REQUIRE(*d == 34.0);
    REQUIRE(foo->x == 42);
    REQUIRE(foo->y == *d);
    REQUIRE(foo->p == NULL);
    mem_pool_destroy(&mem_pool);
}
