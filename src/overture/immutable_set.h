#pragma once

#include "alloc.h"
#include "set.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

/// @file
///
/// Immutable set data structure. Immutable sets are represented as sorted arrays, that
/// are placed in a set data structure, ensuring that there are unique.

#define IMMUTABLE_SET_POOL_DEFAULT_CAPACITY 4
#define IMMUTABLE_SET_SMALL_CAPACITY 4

/// Iterates over the elements of an immutable set.
/// @param elem_ty Type of the elements in the immutable set.
/// @param elem Name of the variable holding a pointer to the current element.
/// @param set Expression evaluating to an immutable set.
#define IMMUTABLE_SET_FOREACH(elem_ty, elem, set) \
    for (elem_ty const* elem = (set).elems; elem != (set).elems + (set).elem_count; ++elem)

/// Declares and implements an immutable set pool, and immutable set functions.
/// @param name Name of the structure representing the immutable set.
/// @param elem_ty Type of the elements in the immutable set.
/// @param hash Hash function with signature `uint32 (uint32_t, const elem_ty*)`
/// @param cmp Comparison function with signature `int (const elem_ty*, const elem_ty*)`.
///   This function returns:
///     - A number > 0 if the left element is greater than the right one,
///     - A number < 0 if the left element is smaller than the right one,
///     - Zero otherwise.
/// @param vis Visibility of the implementation.
/// @see VISIBILITY, IMMUTABLE_SET_DECL, IMMUTABLE_SET_IMPL.
#define IMMUTABLE_SET_DEFINE(name, elem_ty, hash, cmp, vis) \
    IMMUTABLE_SET_DECL(name, elem_ty, vis) \
    IMMUTABLE_SET_IMPL(name, elem_ty, hash, cmp, vis)

/// Declares an immutable set pool, and immutable set functions. Typically used in header files.
/// @see IMMUTABLE_SET_DEFINE.
#define IMMUTABLE_SET_DECL(name, elem_ty, vis) \
    struct name { \
        size_t elem_count; \
        elem_ty elems[]; \
    }; \
    SET_DECL(name##_set, struct name*, vis) \
    struct name##_pool { \
        struct name##_set set; \
    }; \
    [[nodiscard]] VISIBILITY(vis) struct name##_pool name##_pool_create(void); \
    VISIBILITY(vis) void name##_pool_destroy(struct name##_pool*); \
    VISIBILITY(vis) const struct name* name##_pool_insert(struct name##_pool*, elem_ty*, size_t); \
    VISIBILITY(vis) const struct name* name##_pool_insert_unsafe(struct name##_pool*, elem_ty const*, size_t); \
    VISIBILITY(vis) const struct name* name##_pool_merge(struct name##_pool*, const struct name*, const struct name*); \
    VISIBILITY(vis) void name##_pool_reset(struct name##_pool*); \
    VISIBILITY(vis) elem_ty const* name##_find(const struct name*, elem_ty const*); \

/// Implements an immutable set pool, and immutable set functions. Typically used in source files.
/// @see IMMUTABLE_SET_DEFINE.
#define IMMUTABLE_SET_IMPL(name, elem_ty, hash, cmp, vis) \
    static inline int name##_cmp_wrapper(const void* left, const void* right) { \
        return cmp((elem_ty const*)left, (elem_ty const*)right); \
    } \
    static inline bool name##_is_equal(struct name* const* left, struct name* const* right) { \
        if ((*left)->elem_count != (*right)->elem_count) \
            return false; \
        for (size_t i = 0; i < (*left)->elem_count; ++i) { \
            if (cmp(&(*left)->elems[i], &(*right)->elems[i]) != 0) \
                return false; \
        } \
        return true; \
    } \
    static inline uint32_t name##_hash_wrapper(uint32_t h, struct name* const* set) { \
        h = hash_uint64(h, (*set)->elem_count); \
        for (size_t i = 0; i < (*set)->elem_count; ++i) \
            h = hash(h, &(*set)->elems[i]); \
        return h; \
    } \
    static inline struct name* name##_alloc(elem_ty const* elems, size_t elem_count) { \
        struct name* set = xmalloc(sizeof(struct name) + sizeof(elem_ty) * elem_count); \
        set->elem_count = elem_count; \
        memcpy(set->elems, elems, sizeof(elem_ty) * elem_count); \
        return set; \
    } \
    SET_IMPL(name##_set, struct name*, name##_hash_wrapper, name##_is_equal, vis) \
    VISIBILITY(vis) struct name##_pool name##_pool_create(void) { \
        return (struct name##_pool) { .set = name##_set_create() }; \
    } \
    VISIBILITY(vis) void name##_pool_destroy(struct name##_pool* pool) { \
        name##_pool_reset(pool); \
        name##_set_destroy(&pool->set); \
    } \
    VISIBILITY(vis) const struct name* name##_pool_insert(struct name##_pool* pool, elem_ty* elems, size_t elem_count) { \
        qsort(elems, elem_count, sizeof(elem_ty), name##_cmp_wrapper); \
        size_t unique_elem_count = 0; \
        for (size_t i = 0; i < elem_count; ++i) { \
            while (i + 1 < elem_count && elems[i] == elems[i + 1]) i++; \
            elems[unique_elem_count++] = elems[i]; \
        } \
        return name##_pool_insert_unsafe(pool, elems, unique_elem_count); \
    } \
    VISIBILITY(vis) const struct name* name##_pool_insert_unsafe( \
        struct name##_pool* pool, \
        elem_ty const* elems, \
        size_t elem_count) \
    { \
        struct { size_t elem_count; elem_ty elems[IMMUTABLE_SET_SMALL_CAPACITY]; } small_set; \
        struct name* set = (struct name*)&small_set; \
        struct name* set_heap = NULL; /* avoid -Wreturn-local-addr */ \
        if (elem_count <= IMMUTABLE_SET_SMALL_CAPACITY) { \
            memcpy(set->elems, elems, sizeof(elem_ty) * elem_count); \
            set->elem_count = elem_count; \
        } else { \
            set = set_heap = name##_alloc(elems, elem_count); \
        } \
        \
        struct name* const* set_ptr = name##_set_find(&pool->set, &set); \
        if (set_ptr) { \
            free(set_heap); \
            return *set_ptr; \
        } \
        \
        if (!set_heap) \
            set_heap = name##_alloc(elems, elem_count); \
        [[maybe_unused]] bool was_inserted = name##_set_insert(&pool->set, &set_heap); \
        assert(was_inserted); \
        return set_heap; \
    } \
    VISIBILITY(vis) const struct name* name##_pool_merge( \
        struct name##_pool* pool, \
        const struct name* first_set, \
        const struct name* second_set) \
    { \
        elem_ty small_elem_buffer[IMMUTABLE_SET_SMALL_CAPACITY * 2]; \
        elem_ty* merged_elems = small_elem_buffer; \
        if (first_set->elem_count + second_set->elem_count > IMMUTABLE_SET_SMALL_CAPACITY * 2) \
            merged_elems = xmalloc(sizeof(elem_ty) * (first_set->elem_count + second_set->elem_count)); \
        size_t i = 0, j = 0, k = 0; \
        for (; i < first_set->elem_count && j < second_set->elem_count;) { \
            int order = cmp(&first_set->elems[i], &second_set->elems[j]); \
            if (order <= 0) { \
                merged_elems[k++] = first_set->elems[i++]; \
                if (order == 0) \
                    j++; \
            } else { \
                merged_elems[k++] = second_set->elems[j++]; \
            } \
        } \
        for (; i < first_set->elem_count; i++) \
            merged_elems[k++] = first_set->elems[i]; \
        for (; j < second_set->elem_count; j++) \
            merged_elems[k++] = second_set->elems[j]; \
        const struct name* merged_set = name##_pool_insert_unsafe(pool, merged_elems, k); \
        if (merged_elems != small_elem_buffer) \
            free(merged_elems); \
        return merged_set; \
    } \
    VISIBILITY(vis) void name##_pool_reset(struct name##_pool* pool) { \
        SET_FOREACH(struct name*, set_ptr, pool->set) \
            free(*set_ptr); \
        name##_set_clear(&pool->set); \
    } \
    VISIBILITY(vis) elem_ty const* name##_find(const struct name* set, elem_ty const* elem) { \
        return bsearch(elem, set->elems, set->elem_count, sizeof(elem_ty), name##_cmp_wrapper); \
    }
