#include "str_pool.h"
#include "mem_pool.h"
#include "alloc.h"
#include "set.h"

#include <string.h>

SET_DEFINE(str_view_set, struct str_view, str_view_hash, str_view_is_equal, PRIVATE)

struct str_pool {
    struct mem_pool mem_pool;
    struct str_view_set str_view_set;
};

struct str_pool* str_pool_create(void) {
    struct str_pool* str_pool = xmalloc(sizeof(struct str_pool));
    str_pool->mem_pool = mem_pool_create();
    str_pool->str_view_set = str_view_set_create();
    return str_pool;
}

void str_pool_destroy(struct str_pool* str_pool) {
    mem_pool_destroy(&str_pool->mem_pool);
    str_view_set_destroy(&str_pool->str_view_set);
    free(str_pool);
}

const char* str_pool_insert(struct str_pool* str_pool, const char* str) {
    return str_pool_insert_view(str_pool, STR_VIEW(str));
}

const char* str_pool_insert_view(struct str_pool* str_pool, struct str_view str_view) {
    const struct str_view* found = str_view_set_find(&str_pool->str_view_set, &str_view);
    if (found)
        return found->data;

    char* data = mem_pool_alloc(&str_pool->mem_pool, str_view.length + 1, 1);
    memcpy(data, str_view.data, str_view.length);
    data[str_view.length] = 0;
    str_view_set_insert(&str_pool->str_view_set, &(struct str_view) { .data = data, .length = str_view.length });
    return data;
}
