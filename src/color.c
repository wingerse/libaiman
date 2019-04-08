#include <libaiman/color.h>
#include <unistd.h>
#include <stdio.h>

const char *color_if_atty(const char *s, FILE *std)
{
    if (isatty(fileno(std))) {
        return s;
    } else {
        return "";
    }
}