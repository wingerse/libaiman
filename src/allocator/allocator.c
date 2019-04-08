#include <libaiman/allocator/allocator.h>
#include <stdlib.h>

Mallocator mallocator;

void *Mallocator_alloc(Mallocator *a, size_t s)
{
    return malloc(s);
}

void *Mallocator_realloc(Mallocator *a, void *x, size_t s)
{
    return realloc(x, s);
}

void Mallocator_free(Mallocator *a, void *x)
{
    free(x);
}