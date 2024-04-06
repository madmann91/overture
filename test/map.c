#include <overture/test_macros.h>
#include <overture/map.h>

static inline uint32_t hash_int(uint32_t h, const int* i) { return hash_uint32(h, *i); }
static inline bool is_int_equal(const int* i, const int* j) { return *i == *j; }

MAP_DEFINE(int_map, int, int, hash_int, is_int_equal, PRIVATE)

TEST(map) {
    const int n = 100;
    struct int_map int_map = int_map_create();
    for (int i = 0; i < n; ++i)
        REQUIRE(int_map_insert(&int_map, &i, &i));
    REQUIRE(int_map.elem_count == (size_t)n);
    for (int i = 0; i < n; ++i) {
        REQUIRE(int_map_find(&int_map, &i));
        REQUIRE(*int_map_find(&int_map, &i) == i);
    }
    for (int i = 0; i < n; ++i)
        REQUIRE(int_map_remove(&int_map, &i));
    REQUIRE(int_map.elem_count == 0);
    for (int i = 0; i < n; ++i)
        REQUIRE(!int_map_find(&int_map, &i));
    int_map_destroy(&int_map);
}
