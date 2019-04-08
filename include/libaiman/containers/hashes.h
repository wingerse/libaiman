#pragma once
#include "../prelude.h"

inline bool Hashmap_string_eq(const string *a, const string *b)
{
    return string_eq(*a, *b);
}

inline u32 string_hash(const string *k)
{
    u32 hash = 7;
    for (size_t i = 0; i < k->len; i++) {
        hash = 31 * hash + (u32)k->ptr[i];
    }
    return hash;
}