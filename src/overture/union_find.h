#pragma once

#include <stddef.h>

/**
 * @file
 *
 * Union-find algorithm. This union-find algorithm works with an array of integers representing the
 * links between the elements.
 */

/// @return The representative for the given element. This may modify the array during the search.
static inline size_t union_find(size_t* parents, size_t x) {
    size_t y = x;
    while (parents[y] != y)
        y = parents[y];

    while (parents[x] != x) {
        size_t parent = parents[x];
        parents[x] = y;
        x = parent;
    }

    return y;
}

/// Merge element one element into another. After a call to `union_merge(p, x, y)`, a call to
/// `union_find(p, x)` returns the same value as a call to `union_find(p, y)`.
static inline void union_merge(size_t* parents, size_t x, size_t y) {
    x = union_find(parents, x);
    y = union_find(parents, y);
    parents[x] = y;
}
