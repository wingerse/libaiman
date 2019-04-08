#ifdef UNIT_TEST
#include "containers/hashmap.h"
#include "containers/list.h"
#include "allocator/arena.h"
#include <libaiman/unittest.h>

static int run_tests(void)
{
    arena_test();
    list_test();
    hashmap_test();
    return utend();
}

int main(void) 
{
    return run_tests();
}
#endif
typedef int make_iso_compilers_happy; // Wempty-translation-unit