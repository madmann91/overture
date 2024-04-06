#include "mem_pool.h"
#include "alloc.h"

#include <stdlib.h>

#define MIN_BLOCK_CAPACITY 1024

struct mem_block {
    size_t capacity;
    size_t size;
    struct mem_block* next;
    alignas(max_align_t) char data[];
};

struct mem_pool mem_pool_create(void) {
    return (struct mem_pool) {};
}

void mem_pool_destroy(struct mem_pool* mem_pool) {
    for (struct mem_block* block = mem_pool->first; block;) {
        struct mem_block* next = block->next;
        free(block);
        block = next;
    }
}

void mem_pool_reset(struct mem_pool* mem_pool) {
    for (struct mem_block* block = mem_pool->first; block; block = block->next)
        block->size = 0;
    mem_pool->cur = mem_pool->first;
}

static inline size_t align_size(size_t size, size_t align) {
    size_t rem = size % align;
    return rem == 0 ? size : size + align - rem;
}

static inline struct mem_block* alloc_block(size_t capacity) {
    struct mem_block* block = xmalloc(sizeof(struct mem_block) + capacity);
    block->size = 0;
    block->capacity = capacity;
    block->next = NULL;
    return block;
}

void* mem_pool_alloc(struct mem_pool* mem_pool, size_t size, size_t align) {
    struct mem_block* block = mem_pool->cur;
    while (!block || align_size(block->size, align) + size > block->capacity) {
        if (!block || !block->next) {
            size_t capacity = block ? block->capacity : MIN_BLOCK_CAPACITY;
            block = alloc_block(capacity < size ? size : capacity);
            struct mem_block** prev = mem_pool->first ? &mem_pool->cur->next : &mem_pool->first;
            *prev = mem_pool->cur = block;
            break;
        }
        block = block->next;
        mem_pool->cur = block;
    }

    block->size = align_size(block->size, align);
    void* ptr = (void*)(block->data + block->size);
    block->size += size;
    return ptr;
}
