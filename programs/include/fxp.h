#pragma once
#include <stdint.h>
#include "go-board.h"

#define FRAC_BITS 16u
#define FRAC_MASK ((1 << FRAC_BITS) - 1)
#define FXP_ONE  ((fxp32_t)(1 << FRAC_BITS))
#define FXP_ZERO ((fxp32_t)0)

typedef int32_t fxp32_t;

static inline fxp32_t float_to_fxp(float x) {
    return (fxp32_t)(x * FXP_ONE);
}

static inline float fxp_to_float(fxp32_t x) {
    return ((float)x / FXP_ONE);
}

static inline fxp32_t int32_to_fxp(int32_t x) {
    return x << FRAC_BITS;
}

static inline int32_t fxp_to_int32(fxp32_t x) {
    return x >> FRAC_BITS;
}

static inline fxp32_t fxp_min(fxp32_t a, fxp32_t b) {
    return a < b ? a : b;
}

static inline fxp32_t fxp_max(fxp32_t a, fxp32_t b) {
    return a > b ? a : b;
}

static inline fxp32_t fxp_clamp(fxp32_t x, fxp32_t lo, fxp32_t hi) {
    return fxp_min(fxp_max(x, lo), hi);
}

static inline fxp32_t fxp_abs(fxp32_t x) {
    fxp32_t m = x >> 31;
    return (x ^ m) - m;
}

static inline fxp32_t fxp_mul(fxp32_t a, fxp32_t b) {
    int64_t p = (int64_t)a * (int64_t)b;
    return (fxp32_t)(p >> FRAC_BITS);
}

static inline fxp32_t fxp_div(fxp32_t a, fxp32_t b) {
    int64_t p = ((int64_t)a << FRAC_BITS);
    return (fxp32_t)(p / (int64_t)b);
}

static inline uint32_t isqrt_u64(uint64_t x) {
    uint64_t op = x;
    uint64_t res = 0;
    uint64_t one = (uint64_t)1 << 62;

    while (one > op) one >>= 2;

    while (one != 0) {
        if (op >= res + one) {
            op -= res + one;
            res = (res >> 1) + one;
        } else {
            res >>= 1;
        }
        one >>= 2;
    }
    return (uint32_t)res;
}

_fast static fxp32_t fxp_sqrt(fxp32_t x) {
    if (x <= 0) return 0;
    return (fxp32_t)isqrt_u64(((uint64_t)x) << FRAC_BITS);
}
