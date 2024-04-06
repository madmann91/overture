#include <overture/test.h>
#include <overture/heap.h>
#include <overture/minstd.h>
#include <overture/alloc.h>

#include <stdlib.h>

static inline bool less_than_int(const void* i, const void* j) { return *(int*)i < *(int*)j; }

TEST(heap) {
    size_t count = 60;
    uint32_t rng = 42;
    int* values = xmalloc(sizeof(int) * count);

    for (size_t i = 0; i < count; ++i) {
        int value = minstd_gen(&rng) % count;
        heap_push(values, i, sizeof(int), &value, less_than_int);
    }

    for (size_t i = count; i > 0; --i) {
        int value = values[0];
        heap_pop(values, i, sizeof(int), less_than_int);
        values[i - 1] = value;
    }

    for (size_t i = 1; i < count; ++i)
        REQUIRE(values[i - 1] <= values[i]);

    free(values);
}

TEST(heap_sort) {
    size_t count = 60;
    uint32_t rng = 42;
    int* values = xmalloc(sizeof(int) * count);

    for (size_t i = 0; i < count; ++i) {
        values[i] = minstd_gen(&rng) % count;
    }

    heap_sort(values, count, sizeof(int), less_than_int);

    for (size_t i = 1; i < count; ++i)
        REQUIRE(values[i - 1] <= values[i]);

    free(values);
}
