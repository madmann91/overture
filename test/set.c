#include <overture/test_macros.h>
#include <overture/set.h>
#include <overture/minstd.h>

static inline uint32_t hash_int(uint32_t h, const int* i) { return hash_uint32(h, *i); }
static inline bool is_int_equal(const int* i, const int* j) { return *i == *j; }

SET_DEFINE(int_set, int, hash_int, is_int_equal, PRIVATE)

TEST(set) {
    const int n = 100;
    struct int_set int_set = int_set_create();
    int* elems = malloc(sizeof(int) * n);
    for (int i = 0; i < n; ++i) {
        REQUIRE(int_set_insert(&int_set, &i));
        elems[i] = i;
    }
    uint32_t state = 1;
    for (int i = 0; i < n; ++i) {
        int j = minstd_gen(&state) % n;
        int k = minstd_gen(&state) % n;
        int tmp = elems[j];
        elems[j] = elems[k];
        elems[k] = tmp;
    }
    REQUIRE(int_set.elem_count == (size_t)n);
    for (int i = 0; i < n; ++i) {
        REQUIRE(int_set_find(&int_set, &i));
        REQUIRE(*int_set_find(&int_set, &i) == i);
    }
    for (int i = 0; i < n; ++i) {
        REQUIRE(int_set_remove(&int_set, &elems[i]));
        for (int j = i + 1; j < n; ++j)
            REQUIRE(int_set_find(&int_set, &elems[j]));
        REQUIRE(!int_set_find(&int_set, &elems[i]));
    }
    REQUIRE(int_set.elem_count == 0);
    for (int i = 0; i < n; ++i)
        REQUIRE(!int_set_find(&int_set, &i));
    free(elems);
    int_set_destroy(&int_set);
}
