#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "common.h"

void fatal(const char *fmt, ...) {
    printf("fatal: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}