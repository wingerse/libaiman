#pragma once
#include "../prelude.h"
#include "../allocator/allocator.h"

#define LIST_DEFAULT_INITIAL_CAP 4

typedef struct {
    u8 *data;
    size_t len;
    size_t cap;
    size_t elem_size;
    Allocator allocator;
    DestructorFunc *destructor;
} List;

void List_init_custom(List *l, size_t initial_cap, size_t elem_size, Allocator allocator, DestructorFunc *destructor);
void List_init(List *l, size_t elem_size, DestructorFunc *destructor);
void *List_add(List *l);
void *List_get(List *l, size_t i);
bool List_pop(List *l, void *out);
void List_clear(List *l);
void List_reserve(List *l, size_t size);
void List_truncate(List *l, size_t len);
void List_destroy(List *l);
