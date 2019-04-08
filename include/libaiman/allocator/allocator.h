#pragma once
#include "../prelude.h"

#define NEXT_HIGHER_MULTIPLE_POW_2(num, mul) (((num) + ((mul) - 1)) & ~((mul) - 1))

typedef void *AllocFunc(void *allocator, size_t size);
typedef void *ReallocFunc(void *allocator, void *x, size_t size);
typedef void FreeFunc(void *allocator, void *x);
typedef void DestructorFunc(void *x);

typedef struct {
    void *a;
    AllocFunc *alloc_func;
    ReallocFunc *realloc_func;
    FreeFunc *free_func;
} Allocator;

typedef struct { } Mallocator;

extern Mallocator mallocator;

void *Mallocator_alloc(Mallocator *a, size_t s);
void *Mallocator_realloc(Mallocator *a, void *x, size_t s);
void Mallocator_free(Mallocator *a, void *x);

#define MALLOCATOR_ALLOCATOR_FUNC_LIST (AllocFunc *)Mallocator_alloc, (ReallocFunc *)Mallocator_realloc, (FreeFunc *)Mallocator_free