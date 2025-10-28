#pragma once
#include "go-board.h"

static inline float sqrtf(float x) {
    if (x <= 0.0f) return 0.0f;

    float guess = x * 0.5f;
    for (int i = 0; i < 7; i++) {
        guess = 0.5f * (guess + x / guess);
    }
    return guess;
}

static inline float minf(float a, float b) {
    return a > b ? b : a;
}

static inline float maxf(float a, float b) {
    return a >= b ? a : b;
}

static inline float clampf(float n, float l, float h) {
    return n < l ? l : n > h ? h : n;
}
