#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define EXPORT __attribute__((visibility("default")))

#define SQRTF(x) __builtin_sqrtf((x))
#define FABS(x) __builtin_fabs((x))
#define FMIN(x,y) __builtin_fmin((x), (y))
#define SINF(x) __builtin_sinf((x))
#define COSF(x) __builtin_cosf((x))
#define TANF(x) __builtin_tanf((x))

EXPORT void* memcpy(void* dest, const void* restrict src, size_t n) {
    const uint8_t* s = (const uint8_t*) src;
    uint8_t* d = (uint8_t*) dest;
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

EXPORT void* memset(void* str, int c, size_t n) {
    uint8_t* s = (uint8_t*) str;
    for (size_t i = 0; i < n; ++i) {
        s[i] = c;
    }
    return s;
}

#endif // COMMON_H
