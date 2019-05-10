#include <libaiman/unittest.h>
#include <stdarg.h>
#include <libaiman/color.h>

static int failcount;

void utrun_(const char *file, int line, const char *funcname, bool (*func)(void))
{
    printf("TEST  %s:%d:%s\n", file, line, funcname);
    bool success = func();
    if (!success) { failcount++; }
    printf("%s%s%s: %s:%d:%s\n", 
           color_if_atty(success ? SGR(GREEN) : SGR(RED), stdout), 
           success ? "PASS" : "FAIL", 
           color_if_atty(SGR(RESET), stdout), 
           file, 
           line,
           funcname);
}

int utend(void)
{
    printf("\n--- Status: %s%s%s ---\n", 
           color_if_atty(failcount == 0 ? SGR(GREEN) : SGR(RED), stdout),
           failcount == 0 ? "PASS" : "FAIL",
           color_if_atty(SGR(RESET), stdout));
    return !(failcount == 0);
}