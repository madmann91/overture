#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdalign.h>

/// @file
///
/// Low-level heap data structure manipulation functions.

/// Accesses the element at the given index in an untyped heap.
static inline void* heap_at(void* begin, size_t size, size_t i) {
    return ((char*)begin) + size * i;
}

/// Pushes an element on a heap.
/// @param begin Pointer to the beginning of the array.
/// @param count Number of elements to sort.
/// @param size Size of an element.
/// @param elem Pointer to the element to push.
/// @param is_less_than Comparison function.
static inline void heap_push(
    void* begin, size_t count, size_t size,
    const void* elem,
    bool (*is_less_than)(const void*, const void*))
{
    size_t i = count;
    while (i > 0) {
        const size_t parent = (i - 1) / 2;
        const void* parent_elem = heap_at(begin, size, parent);
        if (is_less_than(elem, parent_elem))
            break;
        memcpy(heap_at(begin, size, i), parent_elem, size);
        i = parent;
    }
    memcpy(heap_at(begin, size, i), elem, size);
}

/// Pops an element from a heap.
/// @param begin Pointer to the beginning of the array.
/// @param count Number of elements to sort.
/// @param size Size of an element.
/// @param is_less_than Comparison function.
static inline void heap_pop(
    void* begin, size_t count, size_t size,
    bool (*is_less_than)(const void*, const void*))
{
    size_t i = 0;
    while (true) {
        size_t left  = 2 * i + 1;
        size_t right = 2 * i + 2;
        size_t largest = count - 1;
        if (left < count && is_less_than(heap_at(begin, size, largest), heap_at(begin, size, left)))
            largest = left;
        if (right < count && is_less_than(heap_at(begin, size, largest), heap_at(begin, size, right)))
            largest = right;
        memcpy(heap_at(begin, size, i), heap_at(begin, size, largest), size);
        if (largest == count - 1)
            break;
        i = largest;
    }
}

/// Sorts an array using heap sort.
/// @param begin Pointer to the beginning of the array.
/// @param count Number of elements to sort.
/// @param size Size of an element.
/// @param is_less_than Comparison function.
static inline void heap_sort(
    void* begin, size_t count, size_t size,
    bool (*is_less_than)(const void*, const void*))
{
    alignas(max_align_t) char tmp_elem[size];
    for (size_t i = 0; i < count; ++i) {
        memcpy(tmp_elem, heap_at(begin, size, i), size);
        heap_push(begin, i, size, tmp_elem, is_less_than);
    }
    for (size_t i = count; i > 0; --i) {
        memcpy(tmp_elem, begin, size);
        heap_pop(begin, i, size, is_less_than);
        memcpy(heap_at(begin, size, i - 1), tmp_elem, size);
    }
}
