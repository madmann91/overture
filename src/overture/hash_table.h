#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "primes.h"
#include "alloc.h"

/// @file
///
/// Low-level hash table data structure. When possible, prefer the use of the map or set data
/// structures, as they provide more type safety.

/// Hash table. Can represent both a map or a set.
struct hash_table {
    size_t capacity;    ///< Capacity of the hash table, in number of elements.
    uint32_t* hashes;   ///< Hashes of the keys, with one bit reserved for an occupancy flag.
    char* keys;         ///< Hash table keys.
    char* vals;         ///< Hash table values. May be `NULL`.
};

#define HASH_TABLE_OCCUPIED_FLAG UINT32_C(0x80000000)
#define HASH_TABLE_MAX_LOAD_FACTOR 70 //%

/// Creates a hash table.
/// @param key_size Size of a key (in bytes)
/// @param val_size Size of a value (in bytes)
/// @param init_capacity Initial capacity (in number of elements)
[[nodiscard]] static inline struct hash_table hash_table_create(size_t key_size, size_t val_size, size_t init_capacity) {
    init_capacity = next_prime(init_capacity);
    char* vals = val_size > 0 ? xmalloc(val_size * init_capacity) : NULL;
    return (struct hash_table) {
        .capacity = init_capacity,
        .hashes = xcalloc(init_capacity, sizeof(uint32_t)),
        .keys = xmalloc(key_size * init_capacity),
        .vals = vals
    };
}

/// Destroys the given hash table.
static inline void hash_table_destroy(struct hash_table* hash_table) {
    free(hash_table->hashes);
    free(hash_table->vals);
    free(hash_table->keys);
    memset(hash_table, 0, sizeof(struct hash_table));
}

/// Retrieves the next hash table bucket after the given index.
static inline size_t hash_table_next_bucket(const struct hash_table* hash_table, size_t bucket_idx) {
    return bucket_idx + 1 < hash_table->capacity ? bucket_idx + 1 : 0;
}

/// @return `true` if the bucket at the given index is occupied, `false` otherwise.
static inline bool hash_table_is_bucket_occupied(const struct hash_table* hash_table, size_t bucket_idx) {
    return (hash_table->hashes[bucket_idx] & HASH_TABLE_OCCUPIED_FLAG) != 0;
}

/// @return `true` if the hash table needs rehashing, `false` otherwise.
static inline bool hash_table_needs_rehash(const struct hash_table* hash_table, size_t elem_count) {
    return elem_count * 100 >= hash_table->capacity * HASH_TABLE_MAX_LOAD_FACTOR;
}

/// Clears the hash table, but keeps the allocated memory around.
static inline void hash_table_clear(struct hash_table* hash_table) {
    memset(hash_table->hashes, 0, sizeof(uint32_t) * hash_table->capacity);
}

/// Rehashes the elements into a hash table with the given capacity. The capacity must be large
/// enough to hold all the elements of the hash table.
static inline void hash_table_rehash(
    struct hash_table* hash_table,
    size_t key_size,
    size_t val_size,
    size_t capacity)
{
    struct hash_table copy = hash_table_create(key_size, val_size, capacity);
    for (size_t i = 0; i < hash_table->capacity; ++i) {
        if (!hash_table_is_bucket_occupied(hash_table, i))
            continue;
        uint32_t hash = hash_table->hashes[i];
        size_t idx = mod_prime(hash, copy.capacity);
        while (hash_table_is_bucket_occupied(&copy, idx))
            idx = hash_table_next_bucket(&copy, idx);
        copy.hashes[idx] = hash;
        memcpy(copy.keys + idx * key_size, hash_table->keys + i * key_size, key_size);
        memcpy(copy.vals + idx * val_size, hash_table->vals + i * val_size, val_size);
    }
    hash_table_destroy(hash_table);
    *hash_table = copy;
}

/// Rehashes the given hash table into a new one with larger capacity computed automatically.
static inline void hash_table_grow(
    struct hash_table* hash_table,
    size_t key_size,
    size_t val_size)
{
    size_t next_capacity = hash_table->capacity < MAX_PRIME
        ? next_prime(hash_table->capacity + 1)
        : hash_table->capacity + (hash_table->capacity >> 1);
    hash_table_rehash(hash_table, key_size, val_size, next_capacity);
}

/// Inserts an element into a hash table.
/// @return `true` if the element could not be inserted because it already existed, `false` otherwise.
/// Note that this code does not rehash the hash table, it is the responsibility of the caller to
/// use @ref hash_table_needs_rehash and @ref hash_table_grow as needed.
static inline bool hash_table_insert(
    struct hash_table* hash_table,
    const void* key,
    const void* val,
    size_t key_size,
    size_t val_size,
    uint32_t hash,
    bool (*is_equal) (const void*, const void*))
{
    hash |= HASH_TABLE_OCCUPIED_FLAG;
    size_t idx = mod_prime(hash, hash_table->capacity);
    for (; hash_table_is_bucket_occupied(hash_table, idx); idx = hash_table_next_bucket(hash_table, idx)) {
        if (hash_table->hashes[idx] == hash && is_equal(hash_table->keys + idx * key_size, key))
            return false;
    }
    hash_table->hashes[idx] = hash;
    memcpy(hash_table->keys + idx * key_size, key, key_size);
    memcpy(hash_table->vals + idx * val_size, val, val_size);
    return true;
}

/// Finds an element in a hash table.
/// @return `true` if the element was found, `false` otherwise.
static inline bool hash_table_find(
    const struct hash_table* hash_table,
    size_t* found_idx,
    const void* key,
    size_t key_size,
    uint32_t hash,
    bool (*is_equal) (const void*, const void*))
{
    hash |= HASH_TABLE_OCCUPIED_FLAG;
    size_t idx = mod_prime(hash, hash_table->capacity);
    for (; hash_table_is_bucket_occupied(hash_table, idx); idx = hash_table_next_bucket(hash_table, idx)) {
        if (hash_table->hashes[idx] == hash && is_equal(hash_table->keys + idx * key_size, key)) {
            *found_idx = idx;
            return true;
        }
    }
    return false;
}

/// Removes an element from a hash table.
/// @return `true` if the element was removed, `false` otherwise (if the element was not found).
static inline bool hash_table_remove(
    struct hash_table* hash_table,
    const void* key,
    size_t key_size,
    size_t val_size,
    uint32_t hash,
    bool (*is_equal) (const void*, const void*))
{
    hash |= HASH_TABLE_OCCUPIED_FLAG;
    size_t idx;
    if (!hash_table_find(hash_table, &idx, key, key_size, hash, is_equal))
        return false;

    size_t next_idx = hash_table_next_bucket(hash_table, idx);
    while (hash_table_is_bucket_occupied(hash_table, next_idx)) {
        uint32_t next_hash = hash_table->hashes[next_idx];
        size_t ideal_next_idx = mod_prime(next_hash, hash_table->capacity);
        if (
            (next_idx > idx && (ideal_next_idx <= idx || ideal_next_idx > next_idx)) ||
            (next_idx < idx && (ideal_next_idx <= idx && ideal_next_idx > next_idx)))
        {
            memcpy(hash_table->keys + idx * key_size, hash_table->keys + next_idx * key_size, key_size);
            memcpy(hash_table->vals + idx * val_size, hash_table->vals + next_idx * val_size, val_size);
            hash_table->hashes[idx] = hash_table->hashes[next_idx];
            idx = next_idx;
        }
        next_idx = hash_table_next_bucket(hash_table, next_idx);
    }
    hash_table->hashes[idx] = 0;
    return true;
}
