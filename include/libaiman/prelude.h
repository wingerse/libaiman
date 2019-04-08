#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdnoreturn.h>
#include <stdalign.h>

// lets you know if a pointer is owning (need to free/destroy).
// only for documentation purposes
#define owning

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;

typedef struct {
    char *ptr;
    size_t len;
} string;

#define string_lit(s) (string){s, sizeof(s)-1}
#define string_alloc(s) (string){strdup(s), sizeof(s)-1}

inline bool string_eq(string a, string b)
{
    return a.len == b.len && memcmp(a.ptr, b.ptr, a.len) == 0;
}

inline string string_slice(string a, size_t start, size_t end)
{
    if (end < start) end = start;
    return (string){a.ptr + start, end - start};
}

#define panic(f, ...) _panic(__FILE__, __LINE__, f, ##__VA_ARGS__)
#define unreachable() panic("unreachable")

noreturn void _panic(const char *file, int line, const char *f, ...);