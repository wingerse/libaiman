#pragma once
#include "../prelude.h"
#include "../allocator/allocator.h"

#define HASHMAP_DEFAULT_INITIAL_CAP 16
#define HASHMAP_DEFAULT_LOAD_FACTOR 0.75

typedef struct HashmapEntry HashmapEntry;
struct HashmapEntry {
    u32 hash;
    HashmapEntry *next;
};

typedef u32 HashmapHashFunc(const void *key);
typedef bool HashmapEqFunc(const void *key_a, const void *key_b);

typedef struct {
    u32 len;
    u32 cap;
    float load_factor;
    u32 threshold;

    size_t key_offset;
    size_t key_size;
    size_t value_offset;
    size_t entry_size;

    HashmapHashFunc *hash_func;
    HashmapEqFunc *eq_func;

    Allocator allocator;

    DestructorFunc *key_destructor;
    DestructorFunc *value_destructor;
    
    HashmapEntry **buckets;
} Hashmap;

#define HASHMAP_INIT_PARAMS(TKey, TValue, hash_func, eq_func) \
sizeof(TKey), alignof(TKey), sizeof(TValue), alignof(TValue),\
(HashmapHashFunc *)hash_func, (HashmapEqFunc *)eq_func

void Hashmap_init_custom(Hashmap *h, u32 initial_cap, float load_factor,
                       size_t key_size, size_t key_alignment, size_t value_size, size_t value_alignment,
                       HashmapHashFunc *hash_func, HashmapEqFunc *eq_func,
                       Allocator allocator,
                       DestructorFunc *key_destructor, DestructorFunc *value_destructor);
void Hashmap_init(Hashmap *h, 
                 size_t key_size, size_t key_alignment, size_t value_size, size_t value_alignment,
                 HashmapHashFunc *hash_func, HashmapEqFunc *eq_func,
                 DestructorFunc *key_destructor, DestructorFunc *value_destructor);
void *HashmapEntry_get_key_ptr(HashmapEntry *e, const Hashmap *h);
void *HashmapEntry_get_value_ptr(HashmapEntry *e, const Hashmap *h);
void *Hashmap_put(Hashmap *h, const void *key);
void Hashmap_put_entry(Hashmap *h, HashmapEntry *entry);
void *Hashmap_get(const Hashmap *h, const void *key);
HashmapEntry *Hashmap_extract(Hashmap *h, const void *key);
bool Hashmap_remove(Hashmap *h, const void *key);
void Hashmap_destroy(Hashmap *h);
