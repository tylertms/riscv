#pragma once
#include "fxp.h"
#include "math.h"

#define FXP_COLOR_SCALE ((fxp32_t)16711680)

typedef struct _vec3 {
    fxp32_t x, y, z;
} _vec3;

static inline _vec3 vec3_mul(_vec3 v, fxp32_t s) {
    return (_vec3){
        .x = fxp_mul(v.x, s),
        .y = fxp_mul(v.y, s),
        .z = fxp_mul(v.z, s),
    };
}

static inline _vec3 vec3_div(_vec3 v, fxp32_t s) {
    return (_vec3){
        .x = fxp_div(v.x, s),
        .y = fxp_div(v.y, s),
        .z = fxp_div(v.z, s),
    };
}

static inline fxp32_t magnitude(_vec3 v) {
    return fxp_sqrt(fxp_mul(v.x, v.x) + fxp_mul(v.y, v.y) + fxp_mul(v.z, v.z));
}

static inline _vec3 normalize(_vec3 v) {
    return vec3_div(v, magnitude(v));
}
