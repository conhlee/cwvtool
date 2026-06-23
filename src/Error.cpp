#include "Error.hpp"

#include <cstdlib>
#include <cstdio>

#include <cstdarg>

static char sBuffer[2048];

void Warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vsnprintf(sBuffer, sizeof(sBuffer), fmt, args);

    fprintf(stderr, "\nWARN: %s\n\n", sBuffer);
    fflush(stderr);

    va_end(args);
}

void Error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vsnprintf(sBuffer, sizeof(sBuffer), fmt, args);

    fprintf(stderr, "\nERROR: %s\n\n", sBuffer);
    fflush(stderr);

    va_end(args);
}

void Panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vsnprintf(sBuffer, sizeof(sBuffer), fmt, args);

    fprintf(stderr, "\nPANIC: %s\n\n", sBuffer);
    fflush(stderr);

    va_end(args);

    exit(1);
    __builtin_unreachable();
}
