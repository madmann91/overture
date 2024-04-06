#pragma once

#include "visibility.h"
#include "heap.h"
#include "vec.h"

#include <assert.h>

/**
 * @file
 *
 * Queue data structure providing fast maximum (or minimum, depending on the provided comparison
 * function) element access.
 * @see heap_push, heap_pop.
 */

/// Declares and implements a queue.
/// @param name Name of the structure representing the queue.
/// @param elem_ty Type of the elements in the queue.
/// @param is_less_than Comparison function with signature `bool (const elem_ty*, const elem_ty*)`.
/// @param vis Visibility of the implementation.
/// @see VISIBILITY, QUEUE_DECL, QUEUE_IMPL.
#define QUEUE_DEFINE(name, elem_ty, is_less_than, vis) \
    QUEUE_DECL(name, elem_ty, vis) \
    QUEUE_IMPL(name, elem_ty, is_less_than, vis)

/// Declares a queue data structure. Typically used in header files.
/// @see QUEUE_DEFINE.
#define QUEUE_DECL(name, elem_ty, vis) \
    VEC_DECL(name##_vec, elem_ty, vis) \
    struct name { \
        struct name##_vec vec; \
    }; \
    [[nodiscard]] VISIBILITY(vis) struct name name##_create(void); \
    VISIBILITY(vis) void name##_destroy(struct name*); \
    VISIBILITY(vis) elem_ty const* name##_top(const struct name*); \
    VISIBILITY(vis) bool name##_is_empty(const struct name*); \
    VISIBILITY(vis) void name##_push(struct name*, elem_ty const*); \
    VISIBILITY(vis) void name##_pop(struct name*); \
    VISIBILITY(vis) void name##_clear(struct name*);

/// Implements a queue data structure. Typically used in source files.
/// @see QUEUE_DEFINE.
#define QUEUE_IMPL(name, elem_ty, is_less_than, vis) \
    static inline bool name##_is_less_than_wrapper(const void* left, const void* right) { \
        return is_less_than((elem_ty const*)left, (elem_ty const*)right); \
    } \
    VEC_IMPL(name##_vec, elem_ty, vis) \
    VISIBILITY(vis) struct name name##_create(void) { \
        return (struct name) { .vec = name##_vec_create() }; \
    } \
    VISIBILITY(vis) void name##_destroy(struct name* queue) { \
        name##_vec_destroy(&queue->vec); \
        memset(queue, 0, sizeof(struct name)); \
    } \
    VISIBILITY(vis) elem_ty const* name##_top(const struct name* queue) { \
        assert(!name##_is_empty(queue)); \
        return &queue->vec.elems[0]; \
    } \
    VISIBILITY(vis) bool name##_is_empty(const struct name* queue) { \
        return queue->vec.elem_count == 0; \
    } \
    VISIBILITY(vis) void name##_push(struct name* queue, elem_ty const* elem) { \
        size_t elem_count = queue->vec.elem_count; \
        name##_vec_resize(&queue->vec, elem_count + 1); \
        heap_push(queue->vec.elems, elem_count, sizeof(elem_ty), elem, name##_is_less_than_wrapper); \
    } \
    VISIBILITY(vis) void name##_pop(struct name* queue) { \
        assert(!name##_is_empty(queue)); \
        heap_pop(queue->vec.elems, queue->vec.elem_count, sizeof(elem_ty), name##_is_less_than_wrapper); \
        name##_vec_pop(&queue->vec); \
    } \
    VISIBILITY(vis) void name##_clear(struct name* queue) { \
        name##_vec_clear(&queue->vec); \
    }
