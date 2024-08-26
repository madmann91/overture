#pragma once

#include <stddef.h>
#include <stdalign.h>

/// @file
///
/// Memory pool that allocates blocks of memory suitable for any object type. The pool can be reset,
/// allowing to re-use its blocks for other objects.

struct mem_block;

/// Memory pool data structure.
struct mem_pool {
    struct mem_block* first;    ///< First memory block in the pool.
    struct mem_block* cur;      ///< Current memory block.
};

/// Allocates memory for an object of the given type.
#define MEM_POOL_ALLOC(pool, T) mem_pool_alloc(&(pool), sizeof(T), alignof(T))

/// Allocates memory for an object of the given type.
#define MEM_POOL_ALLOC_ARRAY(pool, n, T) mem_pool_alloc(&(pool), sizeof(T) * n, alignof(T))

/// Creates an empty memory pool.
[[nodiscard]] struct mem_pool mem_pool_create(void);

/// Destroys a memory pool.
void mem_pool_destroy(struct mem_pool*);

/// Resets a memory pool by setting its next free pointer to the first allocated block.
/// This does not free the memory, but instead makes it available for the following allocations.
void mem_pool_reset(struct mem_pool*);

/// Allocates memory on a pool.
/// @param mem_pool Memory pool to use.
/// @param size Size of the object to allocate (in bytes).
/// @param align Alignment of the object to allocate (in bytes).
void* mem_pool_alloc(struct mem_pool* mem_pool, size_t size, size_t align);
