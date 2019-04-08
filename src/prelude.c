#include <libaiman/prelude.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern inline bool string_eq(string a, string b);
extern inline string string_slice(string a, size_t start, size_t end);

noreturn void _panic(const char *file, int line, const char *f, ...)
{
    fprintf(stderr, "panic at %s:%d: ", file, line);
    va_list v;
    va_start(v, f);
    vfprintf(stderr, f, v);
    va_end(v);
    fputs("\n", stderr);
    fflush(stderr);
    exit(EXIT_FAILURE);
}