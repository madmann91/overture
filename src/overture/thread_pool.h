#pragma once

#include <stddef.h>

struct work_item;

typedef void (*work_fn_t)(struct work_item*, size_t);

struct work_item {
    work_fn_t work_fn;
    struct work_item* next;
};

/// Creates a new thread pool with an empty queue.
/// @param thread_count Number of threads to create in the pool, or 0 to autodetect the number of cores.
struct thread_pool* thread_pool_create(size_t thread_count);

/// Destroys the thread pool, and terminates the worker threads, without waiting for completion.
void thread_pool_destroy(struct thread_pool* thread_pool);

/// @return The number of worker threads contained in the given pool.
size_t thread_pool_size(const struct thread_pool* thread_pool);

/// Enqueues several work items in order on a thread pool, using locks to prevent data races.
void thread_pool_submit(struct thread_pool* thread_pool, struct work_item* first, struct work_item* last);

/// Waits for the given number of enqueued work items to terminate, or all of them if `count == 0`.
/// @return The executed work items for re-use.
struct work_item* thread_pool_wait(struct thread_pool* thread_pool, size_t count);
