#include <overture/test_macros.h>
#include <overture/queue.h>

static inline bool less_than_int(const int* i, const int* j) { return *i < *j; }

QUEUE_DEFINE(int_queue, int, less_than_int, PRIVATE)

TEST(queue) {
    const int n = 100;
    struct int_queue int_queue = int_queue_create();
    for (int i = 0; i < n; ++i)
        int_queue_push(&int_queue, &i);
    REQUIRE(int_queue.vec.elem_count == (size_t)n);
    for (int i = n; i-- > 0;) {
        int top = *int_queue_top(&int_queue);
        REQUIRE(top == i);
        int_queue_pop(&int_queue);
    }
    int_queue_destroy(&int_queue);
}
