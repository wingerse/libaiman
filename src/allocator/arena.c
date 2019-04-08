#include <libaiman/allocator/arena.h>
#include <libaiman/allocator/allocator.h>
#include <stdlib.h>
#include <libaiman/unittest.h>
#include <stdalign.h>

#define ARENA_SIZE 4096

void Arena_init(Arena *a) 
{
    a->head = NULL;
    a->current = NULL;
    a->current_offset = 0;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void *new_node(Arena *a, size_t size, bool first_time)
{
    // padding
    size_t offset = NEXT_HIGHER_MULTIPLE_POW_2(sizeof(ArenaNode), alignof(max_align_t));
    // since malloc returns a pointer aligned for all types, aligning the offsets (which are 0 based) also works.
    ArenaNode *n = malloc(offset + MAX(size, ARENA_SIZE)); 
    n->next = NULL;
    n->size = size;

    if (first_time) { a->head = n; } 
    else { a->current->next = n; }
    a->current = n;
    a->current_start_offset = offset;
    a->current_offset = offset + size;

    return (char *)n + offset;
}

void *Arena_alloc(Arena *a, size_t size)
{
    // first time
    if (a->head == NULL) {
        return new_node(a, size, true);
    } else {
        a->current_offset = NEXT_HIGHER_MULTIPLE_POW_2(a->current_offset, alignof(max_align_t));
        // enough space
        if ((a->current_offset - a->current_start_offset) + size <= a->current->size) {
            void *ptr = (char *)a->current + a->current_offset;
            a->current_offset += size;
            return ptr;
        } else {
            return new_node(a, size, false);
        }
    }
}

void *Arena_realloc(Arena *a, void *x, size_t size)
{
    return Arena_alloc(a, size);
}

void Arena_free(Arena *a)
{
    for (ArenaNode *n = a->head; n != NULL; ) {
        ArenaNode *next = n->next;
        free(n);
        n = next;
    }
}

void Arena_free_individual(Arena *a, void *x)
{
    return;
}

#ifdef UNIT_TEST
static bool test_Arena(void)
{
    Arena a;
    Arena_init(&a);
    int *i = Arena_alloc(&a, sizeof(*i));
    *i = 10000;
    char *c = Arena_alloc(&a, sizeof(*c));
    *c = 55;
    double *d = Arena_alloc(&a, sizeof(*d) * 1000);
    d[0] = 1.5;
    d[999] = 0.5;
    short *s = Arena_alloc(&a, sizeof(*s) * 10000);
    s[0] = 1554;
    s[9999] = 29386;
    utassert(*i == 10000);
    utassert(*c == 55);
    utassert(d[0] == 1.5);
    utassert(d[999] == 0.5);
    utassert(s[0] == 1554);
    utassert(s[9999] == 29386);
    return true;
}

void arena_test(void)
{
    utrun(test_Arena);
}
#endif