#pragma once

#include <stddef.h>

/// @file
///
/// Simple thread pool based on POSIX threads.

struct work_item;

/// User function that processes a work item in the thread pool.
typedef void (*work_fn_t)(struct work_item*, size_t);

/// Work item that can be submitted to the thread pool. This is typically used as a member in a
/// larger data structure which contains user data, such as the following example:
///
/// ```c
/// struct my_work_item {
///     struct work_item item;
///     char* data;
/// };
/// void work_fn(struct work_item* item, size_t thread_id) {
///     struct my_work_item* my_item = (struct my_work_item*)item;
///     // do something with `my_item->data`
/// }
///
/// // later in the program
/// char* my_data = malloc(1000);
/// struct my_work_item work = {
///     .item.work_fn = work_fn,
///     .data = my_data
/// };
/// thread_pool_submit(thread_pool, &my_work_item, &my_work_item);
/// ```
///
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

/// Waits for enqueued work items to terminate.
/// @param count Number of work items to wait for, or all of them if equal to 0.
/// @return The executed work items for re-use.
struct work_item* thread_pool_wait(struct thread_pool* thread_pool, size_t count);
