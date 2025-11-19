#pragma once

#include "str.h"

/// @file
///
/// String pool data structure. In this data structure, strings are stored in a hash set so that they
/// can be compared by their address, and such that inserting twice the same string yields the same
/// pointer.

struct str_pool;
struct mem_pool;

/// Creates an empty string pool.
[[nodiscard]] struct str_pool* str_pool_create(struct mem_pool*);

/// Destroys a string pool.
void str_pool_destroy(struct str_pool*);

/// Finds a `NULL`-terminated string in a string pool.
/// @return If it exists, the `NULL`-terminated string in the pool that is equal to the argument, or NULL otherwise.
const char* str_pool_find(struct str_pool*, const char*);

/// Finds a string view in a string pool.
/// @return If it exists, the `NULL`-terminated string in the pool that is equal to the argument, or NULL otherwise.
const char* str_pool_find_view(struct str_pool*, struct str_view);

/// Inserts a `NULL`-terminated string in a string pool.
/// @return A `NULL`-terminated string allocated from the string pool.
const char* str_pool_insert(struct str_pool*, const char*);

/// Inserts a string view in a string pool.
/// @return A `NULL`-terminated string allocated from the string pool.
const char* str_pool_insert_view(struct str_pool*, struct str_view);
