#pragma once
#include "prelude.h"
#include <stdio.h>

void utrun_(const char *file, int line, const char *funcname, bool (*func)(void));
int utend(void);

#define utassert(x) \
do {\
    if (!(x)) {\
        fprintf(stderr, "      %s:%d: %s\n", __FILE__, __LINE__, #x);\
        return false;\
    }\
} while(0)

#define utrun(func) utrun_(__FILE__, __LINE__, #func, func)