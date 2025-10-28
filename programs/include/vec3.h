#pragma once
#include "math.h"

typedef struct _vec3 {
    float x, y, z;
} _vec3;

static inline void vec3_mul(_vec3* v, float s) {
    v->x *= s;
    v->y *= s;
    v->z *= s;
}

static inline void vec3_div(_vec3* v, float s) {
    vec3_mul(v, 1.f / s);
}

static inline float magnitude(_vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline void normalize(_vec3* v) {
    vec3_div(v, magnitude(*v));
}
