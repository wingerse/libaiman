#pragma once
#include "../prelude.h"
#include <stdalign.h>

typedef struct ArenaNode ArenaNode;
struct ArenaNode {
    ArenaNode *next;
    size_t size;
};

typedef struct {
    ArenaNode *head;
    ArenaNode *current;
    size_t current_start_offset;
    size_t current_offset;
} Arena;

void Arena_init(Arena *a);
void *Arena_alloc(Arena *a, size_t size);
// this just allocs again. this is only there for the allocator interface
void *Arena_realloc(Arena *a, void *x, size_t size);
void Arena_free(Arena *a);
// this does nothing and is only there for the allocator interface
void Arena_free_individual(Arena *a, void *x);

#define ARENA_ALLOCATOR_FUNCS_LIST (AllocFunc *)Arena_alloc, (ReallocFunc *)Arena_realloc, (FreeFunc *)Arena_free_individual