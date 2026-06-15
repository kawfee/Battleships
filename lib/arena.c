/**
 * @file arena.c
 * @author Matthew Getgen
 * @brief Code specific to managing memory arenas.
 * @date 2026-05-27
 */

#include "platforms/platform.h"


#define BSHIP_ARENA_PUSH(arena, type) \
    ((type *)BShip_Arena_Push(arena, sizeof(type)))

#define BSHIP_ARENA_PUSH_ARRAY(arena, type, count) \
    ((type *)BShip_Arena_Push(arena, sizeof(type) * (count)))

#define BSHIP_ARENA_TEMP_BEGIN(arena) \
    BShip_ArenaMark __mark = BShip_ArenaMark_Get(arena)

#define BSHIP_ARENA_TEMP_END(arena) \
    BShip_Arena_Rollback(arena, __mark)

static inline void *BShip_Arena_AlignForward(void *address)
{
    return (void *)(((uint64_t)(address) + 0xf) & 0xfffffffffffffff0);
}

static inline BShip_ArenaBlock *BShip_ArenaBlock_Allocate(size_t size)
{
    // printf("allocation size %ld\n", size);
    BShip_ArenaBlock *block = BShip_Allocate(size);
    if (block == NULL)
    {
        return block;
    }
    block->previous = NULL;
    block->capacity = size - sizeof(BShip_ArenaBlock);
    block->offset = 0;
    return block;
}

void BShip_Arena_Initialize(BShip_Arena *arena, size_t capacity)
{
    if (arena == NULL)
    {
        return;
    }
    size_t size = BSHIP_ARENA_BLOCK_SIZE_DEFAULT;
    while (capacity > size)
    {
        size += size;
    }
    BShip_ArenaBlock *block = BShip_ArenaBlock_Allocate(size);
    arena->first = block;
    arena->current = block;
}

void BShip_Arena_Destroy(BShip_Arena *arena)
{
    if (arena == NULL)
    {
        return;
    }
    BShip_ArenaBlock *block = arena->current;
    while (block != NULL)
    {
        BShip_ArenaBlock *next = block->previous;
        BShip_Deallocate(block);
        block = next;
    }
    arena->first = NULL;
    arena->current = NULL;
}

void *BShip_Arena_Push(BShip_Arena *arena, size_t size)
{
    if (arena == NULL || arena->first == NULL || arena->current == NULL)
    {
        return NULL;
    }

    // printf("size %ld\n", size);
    void *memory = &arena->current->memory[arena->current->offset];
    // printf("memory %p\n", memory);
    void *aligned = BShip_Arena_AlignForward(memory);
    // printf("aligned %p\n", aligned);
    // printf("offset %ld\n", arena->current->offset);
    size_t alignment_offset = (size_t)((uint64_t)aligned - (uint64_t)memory);
    // printf("alignment_offset %ld\n", alignment_offset);
    size_t space = arena->current->capacity - (alignment_offset + arena->current->offset);
    // printf("space %ld\n", space);
    if (space >= size)
    {
        arena->current->offset += alignment_offset + size;
        return aligned;
    }
    size_t new_block_capacity = arena->current->capacity * 2;
    while ((size + sizeof(BShip_ArenaBlock)) > new_block_capacity)
    {
        new_block_capacity += new_block_capacity;
    }
    BShip_ArenaBlock *block = BShip_ArenaBlock_Allocate(new_block_capacity);
    if (block == NULL)
    {
        return block;
    }
    block->previous = arena->current;
    block->offset += size;
    arena->current = block;
    return block->memory;
}

void BShip_Arena_Reset(BShip_Arena *arena)
{
    assert(arena != NULL);
    BShip_ArenaBlock *block = arena->current;
    while (block != NULL && arena->first != block)
    {
        BShip_ArenaBlock *next = block->previous;
        BShip_Deallocate(block);
        block = next;
    }
    arena->current = arena->first;
    arena->first->offset = 0;
}

BShip_ArenaMark BShip_ArenaMark_Get(BShip_Arena *arena)
{
    assert(arena != NULL);
    BShip_ArenaMark mark = {
        .block = arena->current,
        .offset = arena->current->offset,
    };
    return mark;
}

void BShip_Arena_Rollback(BShip_Arena *arena, BShip_ArenaMark mark)
{
    assert(arena != NULL);
    assert(mark.block != NULL);

    BShip_ArenaBlock *block = arena->current;
    while (block != mark.block)
    {
        BShip_ArenaBlock *previous = block->previous;
        BShip_Deallocate(block);
        block = previous;
    }
    block->offset = mark.offset;
    arena->current = block;
}

