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
