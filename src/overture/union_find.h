#pragma once

#include <stddef.h>

/**
 * @file
 *
 * Union-find algorithm. This union-find algorithm works with an array of integers representing the
 * links between the elements.
 */

/// @return The representative for the given element. This may modify the array during the search.
size_t union_find(size_t* parents, size_t);
/// Merge element one element into another. After a call to `union_merge(p, x, y)`, a call to
/// `union_find(p, x)` returns the same value as a call to `union_find(p, y)`.
void union_merge(size_t* parents, size_t x, size_t y);
