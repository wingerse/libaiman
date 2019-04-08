#pragma once
#include <stdio.h>

#define CSI "\33["
#define SGR(param) CSI param "m"
#define RESET "0"
#define BOLD "1"
#define RED "31"
#define GREEN "32"

const char *color_if_atty(const char *s, FILE *std);