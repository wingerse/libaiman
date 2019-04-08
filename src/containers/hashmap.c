#include <libaiman/containers/hashmap.h>
#include <libaiman/unittest.h>

void Hashmap_init_custom(Hashmap *h, u32 initial_cap, float load_factor,
                       size_t key_size, size_t key_alignment, size_t value_size, size_t value_alignment,
                       HashmapHashFunc *hash_func, HashmapEqFunc *eq_func,
                       Allocator allocator,
                       DestructorFunc *key_destructor, DestructorFunc *value_destructor)
{
    h->len = 0;
    u32 cap = 1;
    while (cap < initial_cap) {
        cap *= 2;
    }
    h->cap = cap;
    h->load_factor = load_factor;
    h->threshold = (u32)(load_factor * h->cap);

    h->key_size = key_size;
    h->key_offset = NEXT_HIGHER_MULTIPLE_POW_2(sizeof(HashmapEntry), key_alignment);
    h->value_offset = NEXT_HIGHER_MULTIPLE_POW_2(h->key_offset + h->key_size, value_alignment);
    h->entry_size = h->value_offset + value_size;

    h->hash_func = hash_func;
    h->eq_func = eq_func;

    h->allocator = allocator;

    h->key_destructor = key_destructor;
    h->value_destructor = value_destructor;
    
    h->buckets = NULL;
}

void Hashmap_init(Hashmap *h, 
                 size_t key_size, size_t key_alignment, size_t value_size, size_t value_alignment,
                 HashmapHashFunc *hash_func, HashmapEqFunc *eq_func,
                 DestructorFunc *key_destructor, DestructorFunc *value_destructor)
{
    Hashmap_init_custom(h, HASHMAP_DEFAULT_INITIAL_CAP, HASHMAP_DEFAULT_LOAD_FACTOR,
                      key_size, key_alignment, value_size, value_alignment,
                      hash_func, eq_func,
                      (Allocator){&mallocator, MALLOCATOR_ALLOCATOR_FUNC_LIST},
                      key_destructor, value_destructor);
}

static u32 hash(const Hashmap *h, const void *key) 
{
    // magic from jdk 7 hashmap. mitigates problems of power of 2 hashmap sizes
    u32 hsh = h->hash_func(key);
    hsh ^= (hsh >> 20) ^ (hsh >> 12);
    return hsh ^ (hsh >> 7) ^ (hsh >> 4);
}

static void resize(Hashmap *h)
{
    Hashmap new  = *h;
    new.cap *= 2;
    new.threshold = (u32)(new.load_factor * new.cap);
    new.buckets = h->allocator.alloc_func(h->allocator.a, new.cap * sizeof(*new.buckets));
    for (u32 i = 0; i < new.cap; i++) {
        new.buckets[i] = NULL;
    }

    for (u32 i = 0; i < h->cap; i++) {
        HashmapEntry *e = h->buckets[i];
        while (e != NULL) {
            HashmapEntry *next = e->next;
            u32 newHash = e->hash & (new.cap - 1);
            e->next = new.buckets[newHash];
            new.buckets[newHash] = e;
            e = next;
        }
    }

    h->allocator.free_func(h->allocator.a, h->buckets);
    *h = new;
}

static void resize_if_required(Hashmap *h)
{
    // first time
    if (h->buckets == NULL) {
        h->buckets = h->allocator.alloc_func(h->allocator.a, h->cap * sizeof(*h->buckets));
        for (u32 i = 0; i < h->cap; i++) {
            h->buckets[i] = NULL;
        }
    }
    else {
        if (h->len >= h->threshold) {
            resize(h);
        }
    }
}

void *HashmapEntry_get_key_ptr(HashmapEntry *e, const Hashmap *h)
{
    return (char *)e + h->key_offset;
}

void *HashmapEntry_get_value_ptr(HashmapEntry *e, const Hashmap *h)
{
    return (char *)e + h->value_offset;
}

void *Hashmap_put(Hashmap *h, const void *key)
{
    resize_if_required(h);
    u32 hsh = hash(h, key);
    u32 index = hsh & (h->cap - 1);
    HashmapEntry **e = &h->buckets[index];
    for (; *e != NULL; e = &(*e)->next) {
        if ((*e)->hash == hsh && h->eq_func(HashmapEntry_get_key_ptr(*e, h), key)) {
            return HashmapEntry_get_value_ptr(*e, h);
        }
    }
    HashmapEntry *new_entry = h->allocator.alloc_func(h->allocator.a, h->entry_size);
    new_entry->hash = hsh;
    memcpy(HashmapEntry_get_key_ptr(new_entry, h), key, h->key_size);
    new_entry->next = NULL;
    *e = new_entry;
    h->len++;
    return HashmapEntry_get_value_ptr(new_entry, h);
}

void Hashmap_put_entry(Hashmap *h, HashmapEntry *entry)
{
    resize_if_required(h);
    u32 hsh = hash(h, HashmapEntry_get_key_ptr(entry, h));
    u32 index = hsh & (h->cap - 1);
    HashmapEntry *e = h->buckets[index];
    HashmapEntry **prev_next = &h->buckets[index];
    HashmapEntry *next = NULL;
    bool destroy = false;
    for (; e != NULL; prev_next = &e->next, e = e->next) {
        if (e->hash == hsh && h->eq_func(HashmapEntry_get_key_ptr(e, h), HashmapEntry_get_key_ptr(entry, h))) {
            next = e->next;
            if (h->key_destructor != NULL) { h->key_destructor(HashmapEntry_get_key_ptr(e, h)); }
            if (h->value_destructor != NULL) { h->value_destructor(HashmapEntry_get_value_ptr(e, h)); }
            h->allocator.free_func(h->allocator.a, e);
            destroy = true;
            break;
        }
    }
    entry->hash = hsh;
    entry->next = next;
    *prev_next = entry;
    if (!destroy) { h->len++; }
    return;
}

void *Hashmap_get(const Hashmap *h, const void *key)
{
    if (h->len == 0) { return NULL; }
    u32 hsh = hash(h, key);
    u32 index = hsh & (h->cap - 1);
    HashmapEntry *e = h->buckets[index];
    for (; e != NULL; e = e->next) {
        if (e->hash == hsh && h->eq_func(HashmapEntry_get_key_ptr(e, h), key)) {
            return HashmapEntry_get_value_ptr(e, h);
        }
    }
    return NULL;
}

HashmapEntry *Hashmap_extract(Hashmap *h, const void *key)
{
    if (h->len == 0) { return NULL; }
    u32 hsh = hash(h, key);
    u32 index = hsh & (h->cap - 1);
    HashmapEntry *e = h->buckets[index];
    HashmapEntry **prev_next = &h->buckets[index];
    for (; e != NULL; prev_next = &e->next, e = e->next) {
        if (e->hash == hsh && h->eq_func(HashmapEntry_get_key_ptr(e, h), key)) {
            *prev_next = e->next;
            h->len--;
            return e;
        }
    }
    return NULL;
}

bool Hashmap_remove(Hashmap *h, const void *key)
{
    HashmapEntry *entry = Hashmap_extract(h, key);
    if (entry) {
        if (h->key_destructor != NULL) { h->key_destructor(HashmapEntry_get_key_ptr(entry, h)); }
        if (h->value_destructor != NULL) { h->value_destructor(HashmapEntry_get_value_ptr(entry, h)); }
        h->allocator.free_func(h->allocator.a, entry);
        return true;
    }
    return false;
}

void Hashmap_destroy(Hashmap *h)
{
    for (u32 i = 0; i < h->cap; i++) {
        HashmapEntry *e = h->buckets[i];
        while (e != NULL) {
            HashmapEntry *next = e->next;
            if (h->key_destructor != NULL) { h->key_destructor(HashmapEntry_get_key_ptr(e, h)); }
            if (h->value_destructor != NULL) { h->value_destructor(HashmapEntry_get_value_ptr(e, h)); }
            h->allocator.free_func(h->allocator.a, e);
            e = next;
        }
    }
    h->allocator.free_func(h->allocator.a, h->buckets);
}

#ifdef UNIT_TEST
static u32 hash_func(short *a) {
    return (u32)*a;
}
static bool eq_func(short *a, short *b)
{
    return *a == *b;
}

static bool test_Hashmap_Allocator(Allocator a)
{
    // test all functions
    Hashmap h;
    Hashmap_init_custom(&h, HASHMAP_DEFAULT_INITIAL_CAP, HASHMAP_DEFAULT_LOAD_FACTOR,
                      sizeof(short), alignof(short), sizeof(char), alignof(char), 
                      (HashmapHashFunc *)hash_func, (HashmapEqFunc *)eq_func,
                      a,
                      NULL, NULL);
    // ouch, my eyes...
    utassert(Hashmap_get(&h, &(short){1}) == NULL);
    *(char *)Hashmap_put(&h, &(short){10}) = 20;
    *(char *)Hashmap_put(&h, &(short){20}) = 30;
    utassert(*(char *)Hashmap_get(&h, &(short){10}) == 20);
    HashmapEntry *e = Hashmap_extract(&h, &(short){20});
    utassert(Hashmap_get(&h, &(short){20}) == NULL);
    *(short *)HashmapEntry_get_key_ptr(e, &h) = 15;
    Hashmap_put_entry(&h, e);
    utassert(*(char *)Hashmap_get(&h, &(short){15}) == 30);
    *(char *)Hashmap_get(&h, &(short){15}) = 50;
    utassert(*(char *)Hashmap_get(&h, &(short){15}) == 50);
    utassert(Hashmap_remove(&h, &(short){15}) == true);
    utassert(h.len == 1 && *(char *)Hashmap_get(&h, &(short){10}) == 20);
    Hashmap_destroy(&h);
    return true;
}

static bool test_Hashmap(void)
{
    return test_Hashmap_Allocator((Allocator){&mallocator, MALLOCATOR_ALLOCATOR_FUNC_LIST});
}

#include <libaiman/allocator/arena.h>

static bool test_Hashmap_different_Allocator(void)
{
    Arena arena;
    Arena_init(&arena);
    return test_Hashmap_Allocator((Allocator){&arena, ARENA_ALLOCATOR_FUNCS_LIST});
    Arena_free(&arena);
}

void hashmap_test(void)
{
    utrun(test_Hashmap);
    utrun(test_Hashmap_different_Allocator);
}
#endif