#pragma once

#include <stddef.h>

/// @file
///
/// Simple thread pool based on POSIX threads.

/// Work item that can be submitted to the thread pool. This is typically used as a member in a
/// larger data structure which contains user data.
struct work_item {
    /// Pointer to the function that is run by the thread pool. It takes a pointer to the work
    /// item, and the index of the thread that runs the function in the thread pool as arguments.
    void (*work_fn)(struct work_item*, size_t);
    /// Pointer to the next item (if any, otherwise `NULL`).
    struct work_item* next;
};

/// Creates a new thread pool with an empty queue.
/// @param thread_count Number of threads to create in the pool, or 0 to autodetect the number of cores.
[[nodiscard]] struct thread_pool* thread_pool_create(size_t thread_count);

/// Destroys the thread pool, and terminates the worker threads, without waiting for completion.
void thread_pool_destroy(struct thread_pool* thread_pool);

/// @return The number of worker threads contained in the given pool.
[[nodiscard]] size_t thread_pool_size(const struct thread_pool* thread_pool);

/// Enqueues several work items in order on a thread pool, using locks to prevent data races.
void thread_pool_submit(
    struct thread_pool* thread_pool,
    struct work_item* first,
    struct work_item* last);

/// Waits for enqueued work items to terminate.
/// @param count Number of work items to wait for, or all of them if equal to 0.
/// @return The executed work items for re-use.
struct work_item* thread_pool_wait(struct thread_pool* thread_pool, size_t count);
