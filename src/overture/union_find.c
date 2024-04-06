#include "union_find.h"

size_t union_find(size_t* parents, size_t x) {
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

void union_merge(size_t* parents, size_t x, size_t y) {
    x = union_find(parents, x);
    y = union_find(parents, y);
    parents[x] = y;
}
