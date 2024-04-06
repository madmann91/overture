#pragma once

#include <stdbool.h>
#include <stddef.h>

/**
 * @file
 *
 * Low-level heap data structure manipulation functions.
 */

/// Pushes an element on a heap.
/// @param begin Pointer to the beginning of the array.
/// @param count Number of elements to sort.
/// @param size Size of an element.
/// @param elem Pointer to the element to push.
/// @param is_less_than Comparison function.
void heap_push(
    void* begin, size_t count, size_t size, const void* elem,
    bool (*is_less_than)(const void*, const void*));

/// Pops an element from a heap.
/// @param begin Pointer to the beginning of the array.
/// @param count Number of elements to sort.
/// @param size Size of an element.
/// @param is_less_than Comparison function.
void heap_pop(
    void* begin, size_t count, size_t size,
    bool (*is_less_than)(const void*, const void*));

/// Sorts an array using heap sort.
/// @param begin Pointer to the beginning of the array.
/// @param count Number of elements to sort.
/// @param size Size of an element.
/// @param is_less_than Comparison function.
void heap_sort(
    void* begin, size_t count, size_t size,
    bool (*is_less_than)(const void*, const void*));
