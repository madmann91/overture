#include <overture/thread_pool.h>
#include <overture/mem.h>
#include <overture/test.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct my_work_item {
    struct work_item item;
    const int* data;
    size_t count;
    int* sums;
};

void work_fn(struct work_item* item, size_t thread_id) {
    struct my_work_item* my_item = (struct my_work_item*)item;
    int sum = 0;
    for (size_t i = 0; i < my_item->count; ++i) {
        sum += my_item->data[i];
    }
    my_item->sums[thread_id] += sum;
}

TEST(thread_pool) {
    static const size_t count = 20;
    int* data = xmalloc(sizeof(int) * 2 * count);
    for (size_t i = 0; i < 2 * count; ++i)
        data[i] = (int)i;

    static const size_t thread_count = 2;

    int sums[thread_count];
    for (size_t i = 0; i < thread_count; ++i)
        sums[i] = 0;

    struct my_work_item last_item;
    struct my_work_item first_item;

    first_item = (struct my_work_item) {
        .item.work_fn = work_fn,
        .item.next = &last_item.item,
        .data = data,
        .count = count,
        .sums = sums
    };

    last_item = (struct my_work_item) {
        .item.work_fn = work_fn,
        .data = data + count,
        .count = count,
        .sums = sums
    };

    struct thread_pool* thread_pool = thread_pool_create(thread_count);
    thread_pool_submit(thread_pool, &first_item.item, &last_item.item);
    thread_pool_wait(thread_pool, 0);
    thread_pool_destroy(thread_pool);

    int sum = 0;
    for (size_t i = 0; i < thread_count; ++i)
        sum += sums[i];

    int ref = 0;
    for (size_t i = 0; i < 2 * count; ++i)
        ref += data[i];

    REQUIRE(sum == ref);
    free(data);
}
