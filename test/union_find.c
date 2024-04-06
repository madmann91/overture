#include <overture/test_macros.h>
#include <overture/union_find.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TEST(union_find) {
    static const size_t parent_count = 16;
    size_t parents[parent_count];

    for (size_t i = 0; i < parent_count; ++i)
        parents[i] = i;

    for (size_t i = 0; i < parent_count; ++i)
        REQUIRE(union_find(parents, i) == i);

    size_t step = 1;
    while (step < parent_count) {
        step *= 2;
        for (size_t i = 0; i < parent_count - step + 1; i += step) {
            union_merge(parents, i + (step - 1), i);
        }
        for (size_t i = 0; i < parent_count - step + 1; i += step) {
            for (size_t j = i; j < i + step; ++j)
                REQUIRE(union_find(parents, i) == union_find(parents, j));
        }
    }
}
