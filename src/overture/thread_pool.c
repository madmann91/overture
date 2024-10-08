#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>

#include "thread_pool.h"
#include "mem.h"

#define DEFAULT_THREAD_COUNT 2

struct work_queue {
    struct work_item* first_item; // Where the worker threads take work items from
    struct work_item* last_item;  // Where the client's work items are enqueued
    struct work_item* done_items; // Where finished work items are placed
    size_t done_count;            // The number of items that are finished
    size_t done_target;           // The number of items that are required before the next synchronization
    size_t worked_on;             // The number of items being worked on
    pthread_cond_t avail_cond, done_cond;
    pthread_mutex_t mutex;
};

struct thread_data {
    struct thread_pool* thread_pool;
    struct work_queue* queue;
    size_t thread_id;
};

struct thread_pool {
    pthread_t* threads;
    size_t thread_count;
    bool should_stop;
    struct work_queue queue;
    struct thread_data* thread_data;
};

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
static inline long get_system_thread_count(void) { return sysconf(_SC_NPROCESSORS_ONLN); }
#else
static inline long get_system_thread_count(void) { return 0; }
#endif

static inline size_t detect_system_thread_count(void) {
    const char* nproc_str;
    long thread_count;
    if ((nproc_str = getenv("NPROC")) &&
        (thread_count = strtol(nproc_str, NULL, 10)) > 0)
        return thread_count;
    if ((thread_count = get_system_thread_count()) > 0)
        return thread_count;
    return DEFAULT_THREAD_COUNT;
}

static inline bool waiting_condition(const struct work_queue* queue) {
    return
        (queue->worked_on > 0 || queue->first_item) &&
        (queue->done_target == 0 || queue->done_count < queue->done_target);
}

static void* thread_pool_worker(void* data) {
    struct thread_data* thread_data = data;
    struct work_queue* queue = thread_data->queue;
    struct thread_pool* thread_pool = thread_data->thread_pool;
    size_t thread_id = thread_data->thread_id;
    while (true) {
        pthread_mutex_lock(&queue->mutex);
        while (!queue->first_item) {
            if (thread_pool->should_stop || pthread_cond_wait(&queue->avail_cond, &queue->mutex) != 0)
                goto end;
        }
        struct work_item* item = queue->first_item;
        queue->first_item = item->next;
        if (!queue->first_item) {
            assert(queue->last_item == item);
            queue->last_item = NULL;
        }
        queue->worked_on++;
        pthread_mutex_unlock(&queue->mutex);

        item->work_func(item, thread_id);

        pthread_mutex_lock(&queue->mutex);
        item->next = queue->done_items;
        queue->done_items = item;
        queue->worked_on--;
        queue->done_count++;
        if (!waiting_condition(queue))
            pthread_cond_signal(&queue->done_cond);
        pthread_mutex_unlock(&queue->mutex);
    }
end:
    pthread_mutex_unlock(&queue->mutex);
    return NULL;
}

static inline bool init_work_queue(struct work_queue* queue) {
    if (pthread_cond_init(&queue->avail_cond, NULL) != 0)
        return false;        
    if (pthread_cond_init(&queue->done_cond, NULL) != 0)
        goto cleanup_cond;
    if (pthread_mutex_init(&queue->mutex, NULL) != 0)
        goto cleanup_mutex;
    queue->first_item = NULL;
    queue->last_item = NULL;
    queue->done_items = NULL;
    queue->worked_on = 0;
    queue->done_count = 0;
    queue->done_target = 0;
    return true;
cleanup_mutex:
    pthread_cond_destroy(&queue->done_cond);
cleanup_cond:
    pthread_cond_destroy(&queue->avail_cond);
    return false;
}

static inline void free_work_queue(struct work_queue* queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->avail_cond);
    pthread_cond_destroy(&queue->done_cond);
}

static inline void terminate_threads(struct thread_pool* thread_pool) {
    pthread_mutex_lock(&thread_pool->queue.mutex);
    thread_pool->should_stop = true;
    pthread_cond_broadcast(&thread_pool->queue.avail_cond);
    pthread_mutex_unlock(&thread_pool->queue.mutex);
    for (size_t i = 0, n = thread_pool->thread_count; i < n; ++i)
        pthread_join(thread_pool->threads[i], NULL);
}

struct thread_pool* thread_pool_create(size_t thread_count) {
    if (thread_count == 0)
        thread_count = detect_system_thread_count();
    struct thread_pool* thread_pool = xmalloc(sizeof(struct thread_pool));
    if (!init_work_queue(&thread_pool->queue))
        goto cleanup_queue;
    thread_pool->thread_data = xmalloc(sizeof(struct thread_data) * thread_count);
    thread_pool->threads = xmalloc(sizeof(pthread_t) * thread_count);
    thread_pool->thread_count = thread_count;
    thread_pool->should_stop = false;
    for (size_t i = 0; i < thread_count; ++i) {
        thread_pool->thread_data[i].thread_pool = thread_pool;
        thread_pool->thread_data[i].queue = &thread_pool->queue;
        thread_pool->thread_data[i].thread_id = i;
        if (pthread_create(thread_pool->threads + i, NULL, thread_pool_worker, &thread_pool->thread_data[i]) != 0) {
            thread_pool->thread_count = i;
            goto cleanup_thread;
        }
    }
    return thread_pool;
cleanup_thread:
    terminate_threads(thread_pool);
    free_work_queue(&thread_pool->queue);
    free(thread_pool->threads);
    free(thread_pool->thread_data);
cleanup_queue:
    free(thread_pool);
    return NULL;
}

void thread_pool_destroy(struct thread_pool* thread_pool) {
    terminate_threads(thread_pool);
    free_work_queue(&thread_pool->queue);
    free(thread_pool->threads);
    free(thread_pool->thread_data);
    free(thread_pool);
}

size_t thread_pool_size(const struct thread_pool* thread_pool) {
    return thread_pool->thread_count;
}

void thread_pool_submit(struct thread_pool* thread_pool, struct work_item* first, struct work_item* last) {
#ifndef NDEBUG
    // Ensure that following the links from `first` gives `last` as the last element.
    struct work_item* prev = first;
    while (prev->next) prev = prev->next;
    assert(prev == last);
#endif
    pthread_mutex_lock(&thread_pool->queue.mutex);
    if (thread_pool->queue.last_item) {
        assert(thread_pool->queue.first_item);
        thread_pool->queue.last_item->next = first;
        thread_pool->queue.last_item = last;
    } else {
        assert(!thread_pool->queue.first_item);
        thread_pool->queue.first_item = first;
        thread_pool->queue.last_item  = last;
    }
    if (first == last)
        pthread_cond_signal(&thread_pool->queue.avail_cond);
    else
        pthread_cond_broadcast(&thread_pool->queue.avail_cond);
    pthread_mutex_unlock(&thread_pool->queue.mutex);
}

struct work_item* thread_pool_wait(struct thread_pool* thread_pool, size_t count) {
    struct work_queue* queue = &thread_pool->queue;
    struct work_item* done_items = NULL;
    pthread_mutex_lock(&queue->mutex);
    queue->done_target = count;
    while (true) {
        if (waiting_condition(queue) && pthread_cond_wait(&queue->done_cond, &queue->mutex) == 0)
            continue;
        break;
    }
    done_items = queue->done_items;
    queue->done_items = NULL;
    queue->done_count = 0;
    queue->done_target = 0;
    pthread_mutex_unlock(&queue->mutex);
    return done_items;
}
