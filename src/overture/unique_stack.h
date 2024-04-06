#pragma once

#include "vec.h"
#include "set.h"

/// @file
///
/// Stack-like data structure where elements can only be pushed once, no matter if it has been popped
/// or not. This data structure is built from a vector and a hash set, and makes the traversal of
/// graph-like data structures easier.

/// Declares and implements a unique stack data structure.
/// @param name Name of the structure representing the unique stack.
/// @param elem_ty Type of the elements in the unique stack.
/// @param is_equal Comparison function with signature `bool (const elem_ty*, const elem_ty*)`.
/// @param vis Visibility of the implementation.
/// @see VISIBILITY, UNIQUE_STACK_DECL, UNIQUE_STACK_IMPL.
#define UNIQUE_STACK_DEFINE(name, elem_ty, hash, is_equal, vis) \
    UNIQUE_STACK_DECL(name, elem_ty, vis) \
    UNIQUE_STACK_IMPL(name, elem_ty, hash, is_equal, vis)

/// Declares a unique stack data structure.
/// @see UNIQUE_STACK_DEFINE.
#define UNIQUE_STACK_DECL(name, elem_ty, vis) \
    VEC_DECL(name##_vec, elem_ty, vis) \
    SET_DECL(name##_set, elem_ty, vis) \
    struct name { \
        struct name##_vec vec; \
        struct name##_set set; \
    }; \
    [[nodiscard]] VISIBILITY(vis) struct name name##_create(void); \
    VISIBILITY(vis) void name##_destroy(struct name*); \
    VISIBILITY(vis) bool name##_push(struct name*, elem_ty const*); \
    VISIBILITY(vis) bool name##_is_empty(const struct name*); \
    VISIBILITY(vis) elem_ty* name##_pop(struct name*); \
    VISIBILITY(vis) elem_ty* name##_last(struct name*); \
    VISIBILITY(vis) void name##_clear(struct name*);

/// Implements a unique stack data structure.
/// @see UNIQUE_STACK_DEFINE.
#define UNIQUE_STACK_IMPL(name, elem_ty, hash, is_equal, vis) \
    VEC_IMPL(name##_vec, elem_ty, vis) \
    SET_IMPL(name##_set, elem_ty, hash, is_equal, vis) \
    [[nodiscard]] VISIBILITY(vis) struct name name##_create(void) { \
        return (struct name) { \
            .vec = name##_vec_create(), \
            .set = name##_set_create(), \
        }; \
    } \
    VISIBILITY(vis) void name##_destroy(struct name* unique_stack) { \
        name##_vec_destroy(&unique_stack->vec); \
        name##_set_destroy(&unique_stack->set); \
        memset(unique_stack, 0, sizeof(struct name)); \
    } \
    VISIBILITY(vis) bool name##_push(struct name* unique_stack, elem_ty const* elem) { \
        if (name##_set_insert(&unique_stack->set, elem)) { \
            name##_vec_push(&unique_stack->vec, elem); \
            return true; \
        } \
        return false; \
    } \
    VISIBILITY(vis) bool name##_is_empty(const struct name* unique_stack) { \
        return name##_vec_is_empty(&unique_stack->vec); \
    } \
    VISIBILITY(vis) elem_ty* name##_pop(struct name* unique_stack) { \
        return name##_vec_pop(&unique_stack->vec); \
    } \
    VISIBILITY(vis) elem_ty* name##_last(struct name* unique_stack) { \
        return name##_vec_last(&unique_stack->vec); \
    } \
    VISIBILITY(vis) void name##_clear(struct name* unique_stack) { \
        name##_vec_clear(&unique_stack->vec); \
        name##_set_clear(&unique_stack->set); \
    }
