#include "heap.h"
#include "alloc.h"

#include <string.h>
#include <stdlib.h>
#include <stdalign.h>

static inline size_t parent_of(size_t i) {
    return (i - 1) / 2;
}

static inline size_t child_of(size_t i, size_t j) {
    return 2 * i + 1 + j;
}

static inline void* elem_at(void* begin, size_t size, size_t i) {
    return ((char*)begin) + size * i;
}

void heap_push(void* begin, size_t count, size_t size, const void* elem, bool (*is_less_than)(const void*, const void*)) {
    size_t i = count;
    while (i > 0) {
        const void* parent = elem_at(begin, size, parent_of(i));
        if (is_less_than(elem, parent))
            break;
        memcpy(elem_at(begin, size, i), parent, size);
        i = parent_of(i);
    }
    memcpy(elem_at(begin, size, i), elem, size);
}

void heap_pop(void* begin, size_t count, size_t size, bool (*is_less_than)(const void*, const void*)) {
    size_t i = 0;
    while (true) {
        size_t left  = child_of(i, 0);
        size_t right = child_of(i, 1);
        size_t largest = count - 1;
        if (left < count && is_less_than(elem_at(begin, size, largest), elem_at(begin, size, left)))
            largest = left;
        if (right < count && is_less_than(elem_at(begin, size, largest), elem_at(begin, size, right)))
            largest = right;
        memcpy(elem_at(begin, size, i), elem_at(begin, size, largest), size);
        if (largest == count - 1)
            break;
        i = largest;
    }
}

void heap_sort(
    void* begin, size_t count, size_t size,
    bool (*is_less_than)(const void*, const void*))
{
    alignas(max_align_t) char tmp_elem[size];
    for (size_t i = 0; i < count; ++i) {
        memcpy(tmp_elem, elem_at(begin, size, i), size);
        heap_push(begin, i, size, tmp_elem, is_less_than);
    }
    for (size_t i = count; i > 0; --i) {
        memcpy(tmp_elem, begin, size);
        heap_pop(begin, i, size, is_less_than);
        memcpy(elem_at(begin, size, i - 1), tmp_elem, size);
    }
}
