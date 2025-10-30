#pragma once
#include "fxp.h"
#include "math.h"

typedef struct _vec3 {
    fxp32_t x, y, z;
} _vec3;

static inline _vec3 vec3_add_vec3(_vec3 a, _vec3 b) {
    return (_vec3){
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
}

static inline _vec3 vec3_sub_vec3(_vec3 a, _vec3 b) {
    return (_vec3){
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
}

static inline _vec3 vec3_mul_fxp(_vec3 v, fxp32_t s) {
    return (_vec3){
        .x = fxp_mul(v.x, s),
        .y = fxp_mul(v.y, s),
        .z = fxp_mul(v.z, s)
    };
}

static inline _vec3 vec3_div_fxp(_vec3 v, fxp32_t s) {
    return (_vec3){
        .x = fxp_div(v.x, s),
        .y = fxp_div(v.y, s),
        .z = fxp_div(v.z, s)
    };
}

static inline _vec3 vec3_mul_vec3(_vec3 a, _vec3 b) {
    return (_vec3){
        .x = fxp_mul(a.x, b.x),
        .y = fxp_mul(a.y, b.y),
        .z = fxp_mul(a.z, b.z)
    };
}

static inline _vec3 vec3_div_vec3(_vec3 a, _vec3 b) {
    return (_vec3){
        .x = fxp_div(a.x, b.x),
        .y = fxp_div(a.y, b.y),
        .z = fxp_div(a.z, b.z)
    };
}

static inline _vec3 vec3_mul_int32(_vec3 v, int32_t s) {
    return (_vec3){
        .x = v.x * s,
        .y = v.y * s,
        .z = v.z * s
    };
}

static inline _vec3 vec3_div_int32(_vec3 v, int32_t s) {
    return (_vec3){
        .x = v.x / s,
        .y = v.y / s,
        .z = v.z / s
    };
}

static inline _vec3 vec3_neg(_vec3 v) {
    return (_vec3){
        .x = -v.x,
        .y = -v.y,
        .z = -v.z
    };
}

static inline fxp32_t vec3_dot(_vec3 a, _vec3 b) {
    return fxp_mul(a.x, b.x) + fxp_mul(a.y, b.y) + fxp_mul(a.z, b.z);
}

static inline fxp32_t vec3_lensqr(_vec3 v) {
    return vec3_dot(v, v);
}

static inline fxp32_t vec3_len(_vec3 v) {
    return fxp_sqrt(vec3_lensqr(v));
}

static inline _vec3 vec3_normalize(_vec3 v) {
    return vec3_div_fxp(v, vec3_len(v));
}

static inline _vec3 vec3_unit_to_uniform01(_vec3 v) {
    return (_vec3){
        .x = (v.x + FXP_ONE) >> 1,
        .y = (v.y + FXP_ONE) >> 1,
        .z = (v.z + FXP_ONE) >> 1
    };
}
