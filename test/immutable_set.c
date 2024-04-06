#include <overture/test.h>
#include <overture/immutable_set.h>

static inline uint32_t hash_int(uint32_t h, const int* i) { return hash_uint32(h, *i); }
static inline int cmp_int(const int* i, const int* j) { return *i - *j; }

IMMUTABLE_SET_DEFINE(immutable_int_set, int, hash_int, cmp_int, PRIVATE)

TEST(immutable_set) {
    struct immutable_int_set_pool int_set_pool = immutable_int_set_pool_create();
    const struct immutable_int_set* one_two_three = immutable_int_set_pool_insert(&int_set_pool, (int[]) { 1, 2, 3 }, 3);
    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 1, 2, 3 }, 3));
    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 1, 2, 3, 1 }, 4));
    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 3, 2, 1 }, 3));
    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 1, 3, 2 }, 3));
    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 2, 3, 1 }, 3));
    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 2, 1, 3 }, 3));
    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 3, 1, 2 }, 3));
    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 3, 1, 2 }, 3));

    const struct immutable_int_set* one_two_three_four =
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 1, 2, 3, 4 }, 4);
    REQUIRE(one_two_three != one_two_three_four);

    const int* one = immutable_int_set_find(one_two_three, (int[]) { 1 });
    const int* two = immutable_int_set_find(one_two_three, (int[]) { 2 });
    const int* three = immutable_int_set_find(one_two_three, (int[]) { 3 });
    REQUIRE(one && *one == 1);
    REQUIRE(two && *two == 2);
    REQUIRE(three && *three == 3);
    REQUIRE(!immutable_int_set_find(one_two_three, (int[]) { 4 }));

    REQUIRE(
        one_two_three ==
        immutable_int_set_pool_merge(&int_set_pool, one_two_three, one_two_three));
    REQUIRE(
        one_two_three_four ==
        immutable_int_set_pool_merge(&int_set_pool, one_two_three, one_two_three_four));
    REQUIRE(
        one_two_three_four ==
        immutable_int_set_pool_merge(&int_set_pool, one_two_three_four, one_two_three));
    const struct immutable_int_set* four =
        immutable_int_set_pool_insert(&int_set_pool, (int[]) { 4 }, 1);
    REQUIRE(
        one_two_three_four ==
        immutable_int_set_pool_merge(&int_set_pool, one_two_three, four));
    REQUIRE(
        one_two_three_four ==
        immutable_int_set_pool_merge(&int_set_pool, four, one_two_three));
    immutable_int_set_pool_destroy(&int_set_pool);
}
