#include <libaiman/containers/list.h>
#include <libaiman/unittest.h>

void List_init_custom(List *l, size_t initial_cap, size_t elem_size, Allocator allocator, DestructorFunc *destructor)
{
    l->data = NULL;
    l->len = 0;
    size_t cap = 1;
    while (cap < initial_cap) {
        cap *= 2;
    }
    l->cap = cap;
    l->elem_size = elem_size;
    l->allocator = allocator;
    l->destructor = destructor;
}

void List_init(List *l, size_t elem_size, DestructorFunc *destructor)
{
    List_init_custom(l, LIST_DEFAULT_INITIAL_CAP, elem_size, (Allocator){&mallocator, MALLOCATOR_ALLOCATOR_FUNC_LIST}, destructor);
}

void *List_add(List *l)
{
    // first time
    if (l->data == NULL) {
        l->data = l->allocator.alloc_func(l->allocator.a, l->cap * l->elem_size);
    } else {
        if (l->len == l->cap) {
            l->cap *= 2;
            l->data = l->allocator.realloc_func(l->allocator.a, l->data, l->cap * l->elem_size);
        }
    }
    void *ptr = &l->data[l->len * l->elem_size];
    l->len++;
    return ptr;
}

void *List_get(List *l, size_t i)
{
    return &l->data[i * l->elem_size];
}

bool List_pop(List *l, void *out)
{
    if (l->len == 0) { return false; }
    l->len--;
    memcpy(out, &l->data[l->len * l->elem_size], l->elem_size); 
    return true;
}

void List_clear(List *l)
{
    if (l->destructor != NULL) {
        for (size_t i = 0; i < l->len; i++) {
            l->destructor(&l->data[i * l->elem_size]);
        }
    }
    l->len = 0;
}

void List_reserve(List *l, size_t size)
{
    if (size > l->cap) {
        size_t cap = 1;
        while (cap < size) {
            cap *= 2;
        }
        l->cap = cap;
        l->data = l->allocator.realloc_func(l->allocator.a, l->data, l->cap * l->elem_size);
    }
}

void List_truncate(List *l, size_t len)
{
    if (len < l->len)
    {
        if (l->destructor != NULL) {
            for (size_t i = len; i < l->len; i++) {
                l->destructor(&l->data[i * l->elem_size]);
            }
        }
        l->len = len;
    }
}

void List_destroy(List *l) 
{
    List_clear(l);
    l->allocator.free_func(l->allocator.a, l->data);
}

#ifdef UNIT_TEST
static bool test_List(void)
{
    List l;
    List_init(&l, sizeof(int), NULL);
    *(int *)List_add(&l) = 100;
    *(int *)List_add(&l) = 200;
    *(int *)List_add(&l) = 300;
    *(int *)List_add(&l) = 400;
    utassert(l.len == 4);
    utassert(l.cap == 4);
    *(int *)List_add(&l) = 500;
    utassert(l.len == 5);
    utassert(l.cap == 8);
    int i;
    utassert(List_pop(&l, &i) == true && i == 500 && l.len == 4 && l.cap == 8);
    List_reserve(&l, 20);
    utassert(l.cap == 32);
    utassert(*(int *)List_get(&l, 0) == 100);
    utassert(*(int *)List_get(&l, 1) == 200);
    utassert(*(int *)List_get(&l, 2) == 300);
    utassert(*(int *)List_get(&l, 3) == 400);
    List_truncate(&l, 2);
    utassert(*(int *)List_get(&l, 0) == 100);
    utassert(*(int *)List_get(&l, 1) == 200);
    utassert(l.len == 2);
    return true;
}

void list_test(void) 
{
    utrun(test_List);
}
#endif