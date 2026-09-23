/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2026 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hglm.h is a (mostly) vector math library (with some SIMD support).
 *
 *
 * USAGE:
 *
 * Include `hglm.h` like this:
 *
 *     #define HGLM_STRIP_PREFIX              // optional
 *     #define HGLM_USE_SIMD                  // optional
 *     #include "hglm.h"
 *
 *
 * EXAMPLE:
 *
 * project vector a onto b (with HGLM_STRIP_PREFIX):
 *
 *     Vec2 a = vec2(10, 5);
 *     Vec2 b = vec2(20, 0);
 *     Vec2 projb_a = vec2_mul_scalar(b, (vec2_dot(a, b) / vec2_dot(b, b)));
 *     vec2_print(projb_a);
 *
 * spherical linear interpolation between a and b (with HGLM_STRIP_PREFIX):
 *
 *     Vec2 a = vec2(10, 0);
 *     Vec2 b = vec2(0, 10);
 *     for (int i = 0; i <= 8; i++) {
 *         vec2_print(vec2_slerp(a, b, (float)i/8));
 *     }
 *     
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGLM_H
#define HGLM_H

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <complex.h>
#include <string.h>
#include <stdio.h>

#include <assert.h> // DEBUG

#define HGL_INLINE inline __attribute__((always_inline))

#if !defined(HGLM_ALLOC) && !defined(HGLM_FREE)
#   include <stdlib.h>
#   define HGLM_ALLOC malloc
#   define HGLM_FREE  free
#endif

#define HGLM_PI 3.14159265358979

#define HGLM_DEG_TO_RAD(deg) ((deg)*(HGLM_PI/180.0f))
#define HGLM_RAD_TO_DEG(rad) ((rad)*(180.0f/HGLM_PI))

#ifdef HGLM_USE_SIMD
#   include <smmintrin.h>
#   include <immintrin.h>
#endif

#define HGLM_MAT2_IDENTITY ((HglmMat2) {   \
    .m00 = 1.0f, .m01 = 0.0f,              \
    .m10 = 0.0f, .m11 = 1.0f,})

#define HGLM_MAT3_IDENTITY ((HglmMat3) {   \
    .m00 = 1.0f, .m01 = 0.0f, .m02 = 0.0f, \
    .m10 = 0.0f, .m11 = 1.0f, .m12 = 0.0f, \
    .m20 = 0.0f, .m21 = 0.0f, .m22 = 1.0f,})

#define HGLM_MAT4_IDENTITY ((HglmMat4) {                \
    .m00 = 1.0f, .m01 = 0.0f, .m02 = 0.0f, .m03 = 0.0f, \
    .m10 = 0.0f, .m11 = 1.0f, .m12 = 0.0f, .m13 = 0.0f, \
    .m20 = 0.0f, .m21 = 0.0f, .m22 = 1.0f, .m23 = 0.0f, \
    .m30 = 0.0f, .m31 = 0.0f, .m32 = 0.0f, .m33 = 1.0f,})

typedef struct
{
    int x;
    int y;
} HglmIVec2;

typedef struct
{
    int x;
    int y;
    int z;
} HglmIVec3;

typedef struct
{
    int x;
    int y;
    int z;
    int w;
} HglmIVec4;

typedef struct
{
    float x;
    float y;
} HglmVec2;

typedef struct
{
    union {
        struct {
            float x;
            float y;
        };
        HglmVec2 xy;
    };
    float z;
} HglmVec3;

typedef union __attribute__ ((aligned(16)))
{
    struct {
        union {
            struct {
                union {
                    struct {
                        float x;
                        float y;
                    };
                    HglmVec2 xy;
                };
                float z;
            };
            HglmVec3 xyz;
        };
        float w;
    };
#ifdef HGLM_USE_SIMD
    __m128 v;
#endif
    float f[4];
} HglmVec4;

typedef HglmVec4 HglmQuat;

typedef struct __attribute__ ((aligned(16)))
{
    union {
        struct {
            HglmVec2 c0;
            HglmVec2 c1;
        };
        struct {
            float m00;
            float m10;
            float m01;
            float m11;
        };
        float f[4];
    };
} HglmMat2;

typedef struct __attribute__ ((aligned(16)))
{
    union {
        struct {
            HglmVec3 c0;
            HglmVec3 c1;
            HglmVec3 c2;
        };
        struct {
            float m00;
            float m10;
            float m20;
            float m01;
            float m11;
            float m21;
            float m02;
            float m12;
            float m22;
        };
        float f[9];
    };
} HglmMat3;

typedef struct __attribute__ ((aligned(16)))
{
    union {
        struct {
            HglmVec4 c0;
            HglmVec4 c1;
            HglmVec4 c2;
            HglmVec4 c3;
        };
        struct {
            float m00;
            float m10;
            float m20;
            float m30;
            float m01;
            float m11;
            float m21;
            float m31;
            float m02;
            float m12;
            float m22;
            float m32;
            float m03;
            float m13;
            float m23;
            float m33;
        };
        float f[16];
    };
} HglmMat4;

typedef struct
{
    float *data;
    union {
        uint32_t M;
        uint32_t rows;
    };
    union {
        uint32_t N;
        uint32_t cols;
    };
} HglmMat;

static HGL_INLINE HglmIVec2 hglm_ivec2(int x, int y);
static HGL_INLINE HglmIVec2 hglm_ivec2_add(HglmIVec2 a, HglmIVec2 b);
static HGL_INLINE HglmIVec2 hglm_ivec2_sub(HglmIVec2 a, HglmIVec2 b);
static HGL_INLINE float hglm_ivec2_distance(HglmIVec2 a, HglmIVec2 b);
static HGL_INLINE float hglm_ivec2_length(HglmIVec2 v);
static HGL_INLINE HglmIVec2 hglm_ivec2_mul_scalar(HglmIVec2 v, float s);
static HGL_INLINE HglmIVec2 hglm_ivec2_lerp(HglmIVec2 a, HglmIVec2 b, float amount);

static HGL_INLINE HglmIVec3 hglm_ivec3(int x, int y, int z);
static HGL_INLINE HglmIVec3 hglm_ivec3_add(HglmIVec3 a, HglmIVec3 b);
// TODO ...

static HGL_INLINE HglmIVec4 hglm_ivec4(int x, int y, int z, int w);
// TODO ...

static HGL_INLINE HglmVec2 hglm_vec2(float x, float y);
static HGL_INLINE HglmVec2 hglm_vec2_from_polar(float r, float phi);
static HGL_INLINE HglmVec2 hglm_vec2_add(HglmVec2 a, HglmVec2 b);
static HGL_INLINE HglmVec2 hglm_vec2_sub(HglmVec2 a, HglmVec2 b);
static HGL_INLINE float hglm_vec2_distance(HglmVec2 a, HglmVec2 b);
static HGL_INLINE float hglm_vec2_length(HglmVec2 v);
static HGL_INLINE HglmVec2 hglm_vec2_normalize(HglmVec2 v);
static HGL_INLINE float hglm_vec2_dot(HglmVec2 a, HglmVec2 b);
static HGL_INLINE float hglm_vec2_wedge(HglmVec2 a, HglmVec2 b); // psuedo 2D cross product. I.e. 2x2 matrix determinant
static HGL_INLINE HglmVec2 hglm_vec2_recip(HglmVec2 v);
static HGL_INLINE HglmVec2 hglm_vec2_hadamard(HglmVec2 a, HglmVec2 b);
static HGL_INLINE HglmVec2 hglm_vec2_mul_scalar(HglmVec2 v, float s);
static HGL_INLINE HglmVec2 hglm_vec2_reflect(HglmVec2 v, HglmVec2 normal);
static HGL_INLINE HglmVec2 hglm_vec2_lerp(HglmVec2 a, HglmVec2 b, float t);
static HGL_INLINE HglmVec2 hglm_vec2_slerp(HglmVec2 a, HglmVec2 b, float t);
static HGL_INLINE HglmVec2 hglm_vec2_bezier3(HglmVec2 v0, HglmVec2 v1, HglmVec2 v2, HglmVec2 v3, float t);

static HGL_INLINE HglmVec3 hglm_vec3(float x, float y, float z);
static HGL_INLINE HglmVec3 hglm_vec3_from_spherical(float r, float phi, float theta);
static HGL_INLINE HglmVec3 hglm_vec3_add(HglmVec3 a, HglmVec3 b);
static HGL_INLINE HglmVec3 hglm_vec3_sub(HglmVec3 a, HglmVec3 b);
static HGL_INLINE float hglm_vec3_distance(HglmVec3 a, HglmVec3 b);
static HGL_INLINE float hglm_vec3_length(HglmVec3 v);
static HGL_INLINE HglmVec3 hglm_vec3_normalize(HglmVec3 v);
static HGL_INLINE float hglm_vec3_dot(HglmVec3 a, HglmVec3 b);
static HGL_INLINE HglmVec3 hglm_vec3_cross(HglmVec3 a, HglmVec3 b);
static HGL_INLINE HglmVec3 hglm_vec3_recip(HglmVec3 v);
static HGL_INLINE HglmVec3 hglm_vec3_hadamard(HglmVec3 a, HglmVec3 b);
static HGL_INLINE HglmVec3 hglm_vec3_mul_scalar(HglmVec3 v, float s);
static HGL_INLINE HglmVec3 hglm_vec3_reflect(HglmVec3 v, HglmVec3 normal);
static HGL_INLINE HglmVec3 hglm_vec3_lerp(HglmVec3 a, HglmVec3 b, float t);
static HGL_INLINE HglmVec3 hglm_vec3_slerp(HglmVec3 a, HglmVec3 b, float t);
static HGL_INLINE HglmVec3 hglm_vec3_bezier3(HglmVec3 v0, HglmVec3 v1, HglmVec3 v2, HglmVec3 v3, float t);

static HGL_INLINE HglmVec4 hglm_vec4(float x, float y, float z, float w);
static HGL_INLINE HglmVec4 hglm_vec4_add(HglmVec4 a, HglmVec4 b);
static HGL_INLINE HglmVec4 hglm_vec4_sub(HglmVec4 a, HglmVec4 b);
static HGL_INLINE float hglm_vec4_distance(HglmVec4 a, HglmVec4 b);
static HGL_INLINE float hglm_vec4_length(HglmVec4 v);
static HGL_INLINE HglmVec4 hglm_vec4_normalize(HglmVec4 v);
static HGL_INLINE float hglm_vec4_dot(HglmVec4 a, HglmVec4 b);
static HGL_INLINE HglmVec4 hglm_vec4_recip(HglmVec4 v);
static HGL_INLINE HglmVec4 hglm_vec4_hadamard(HglmVec4 a, HglmVec4 b);
static HGL_INLINE HglmVec4 hglm_vec4_mul_scalar(HglmVec4 v, float s);
static HGL_INLINE HglmVec4 hglm_vec4_swizzle(HglmVec4 v, int a, int b, int c, int d);
static HGL_INLINE HglmVec4 hglm_vec4_perspective_divide(HglmVec4 v);
static HGL_INLINE HglmVec4 hglm_vec4_lerp(HglmVec4 a, HglmVec4 b, float t);
static HGL_INLINE HglmVec4 hglm_vec4_bezier3(HglmVec4 v0, HglmVec4 v1, HglmVec4 v2, HglmVec4 v3, float t);

static HGL_INLINE HglmQuat hglm_quat(float w, float x, float y, float z);
static HGL_INLINE HglmQuat hglm_quat_from_angle_axis(float angle, HglmVec3 axis);
static HGL_INLINE HglmQuat hglm_quat_mul_quat(HglmQuat a, HglmQuat b);
static HGL_INLINE HglmQuat hglm_quat_mul_scalar(HglmQuat q, float s);
static HGL_INLINE HglmQuat hglm_quat_pow(HglmQuat q, float y);
static HGL_INLINE float hglm_quat_length(HglmQuat q);
static HGL_INLINE float hglm_quat_square_length(HglmQuat q);
static HGL_INLINE HglmQuat hglm_quat_conjugate(HglmQuat q);
static HGL_INLINE HglmQuat hglm_quat_inverse(HglmQuat q);
static HGL_INLINE HglmVec3 hglm_quat_apply_vec3(HglmQuat q, HglmVec3 v);
static HGL_INLINE HglmQuat hglm_quat_slerp(HglmQuat q0, HglmQuat q1, float t, bool shortest_path);

static HGL_INLINE HglmMat2 hglm_mat2(HglmVec2 c0, HglmVec2 c1);
static HGL_INLINE HglmMat2 hglm_mat2_identity(void);
static HGL_INLINE HglmMat2 hglm_mat2_make_scale(HglmVec2 v);
static HGL_INLINE HglmMat2 hglm_mat2_make_rotation(float angle);
static HGL_INLINE HglmMat2 hglm_mat2_adjugate(HglmMat2 m);
static HGL_INLINE HglmMat2 hglm_mat2_inverse(HglmMat2 m);
static HGL_INLINE HglmMat2 hglm_mat2_add(HglmMat2 a, HglmMat2 b);
static HGL_INLINE HglmMat2 hglm_mat2_sub(HglmMat2 a, HglmMat2 b);
static HGL_INLINE HglmMat2 hglm_mat2_mul_scalar(HglmMat2 m, float s);
static HGL_INLINE HglmVec2 hglm_mat2_mul_vec2(HglmMat2 m, HglmVec2 v);
static HGL_INLINE HglmMat2 hglm_mat2_mul_mat2(HglmMat2 a, HglmMat2 b);
static HGL_INLINE HglmMat2 hglm_mat2_scale(HglmMat2 m, HglmVec2 v);
static HGL_INLINE HglmMat2 hglm_mat2_rotate(HglmMat2 m, float angle);
static HGL_INLINE float hglm_mat2_determinant(HglmMat2 m);
// TODO ...

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3(HglmVec3 c0, HglmVec3 c1, HglmVec3 c2);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_identity(void);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_scale(HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_rotation(float angle, HglmVec3 axis);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_translation(HglmVec2 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_demote_from_mat4(HglmMat4 mat4);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_adjugate(HglmMat3 m);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_inverse(HglmMat3 m);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_affine_inverse(HglmMat3 m);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_transpose(HglmMat3 m);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_add(HglmMat3 a, HglmMat3 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_sub(HglmMat3 a, HglmMat3 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_mul_scalar(HglmMat3 m, float s);
__attribute__ ((const, unused)) static HGL_INLINE HglmVec3 hglm_mat3_mul_vec3(HglmMat3 m, HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_mul_mat3(HglmMat3 a, HglmMat3 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_scale(HglmMat3 m, HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_rotate(HglmMat3 m, float angle, HglmVec3 axis);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_translate(HglmMat3 m, HglmVec2 v);
__attribute__ ((const, unused)) static HGL_INLINE float hglm_mat3_determinant(HglmMat3 m);

__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4(HglmVec4 c0, HglmVec4 c1, HglmVec4 c2, HglmVec4 c3);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_zero(void);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_identity(void);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_scale(HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_rotation(float angle, HglmVec3 axis);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_translation(HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_promote_from_mat3(HglmMat3 mat3);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_ortho(float left, float right, float bottom, float top,  float near,  float far);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_ortho_inverse(float left, float right, float bottom, float top,  float near,  float far);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_perspective(float fov, float aspect, float znear, float zfar);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_affine_inverse(HglmMat4 m);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_transpose(HglmMat4 m);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_look_at(HglmVec3 camera, HglmVec3 target, HglmVec3 up);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_look_to(HglmVec3 camera, HglmVec3 dir, HglmVec3 up);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_add(HglmMat4 a, HglmMat4 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_sub(HglmMat4 a, HglmMat4 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_mul_scalar(HglmMat4 m, float s);
__attribute__ ((const, unused)) static HGL_INLINE HglmVec4 hglm_mat4_mul_vec4(HglmMat4 m, HglmVec4 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_mul_mat4(HglmMat4 a, HglmMat4 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_scale(HglmMat4 m, HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_rotate(HglmMat4 m, float angle, HglmVec3 axis);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_translate(HglmMat4 m, HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmVec4 hglm_mat4_perspective_project(HglmMat4 proj, HglmVec4 v);

static HGL_INLINE HglmMat hglm_mat_make(uint32_t M /* rows */, uint32_t N /* cols */);
static HGL_INLINE HglmMat hglm_mat_make_identity(uint32_t N);
static HGL_INLINE HglmMat hglm_mat_make_copy(HglmMat m);
static HGL_INLINE void hglm_mat_free(HglmMat m);
static HGL_INLINE void hglm_mat_fill(HglmMat m, float value);
static HGL_INLINE void hglm_mat_add(HglmMat res, HglmMat a, HglmMat b);
static HGL_INLINE void hglm_mat_sub(HglmMat res, HglmMat a, HglmMat b);
static HGL_INLINE void hglm_mat_mul_scalar(HglmMat m, float s);
static HGL_INLINE void hglm_mat_mul_mat(HglmMat res, HglmMat a, HglmMat b);
static HGL_INLINE void hglm_mat_transpose_in_place(HglmMat m);
static HGL_INLINE void hglm_mat_transpose(HglmMat t, HglmMat m);
static HGL_INLINE float hglm_mat_inverse(HglmMat inv, HglmMat m);

static HGL_INLINE float hglm_pid(float error, float last_error, float *i, 
                                 float Kp, float Ki, float Kd, float dt);
static HGL_INLINE float hglm_lerp(float a, float b, float t);
static HGL_INLINE float hglm_ilerp(float a, float b, float value);
static HGL_INLINE float hglm_clamp(float min, float max, float value);
static HGL_INLINE float hglm_remap(float in_min, float in_max, float out_min, float out_max, float value);
static HGL_INLINE float hglm_smoothstep(float t);
static HGL_INLINE float hglm_smootherstep(float t);
static HGL_INLINE float hglm_sinstep(float t);
static HGL_INLINE float hglm_lerpsmooth(float a, float b, float dt, float omega);
static HGL_INLINE float hglm_smoothmin_quadratic(float a, float b, float k);
static HGL_INLINE float hglm_smoothmin_sigmoid(float a, float b, float k);
static HGL_INLINE HglmVec4 hglm_bezier3f(float t);
static HGL_INLINE HglmVec4 hglm_hermite3f(float t);
static HGL_INLINE float hglm_perlin3D(float x, float y, float z);

static HGL_INLINE void hglm_fft(float in[], float complex out[], int n);
static HGL_INLINE void hglm_ifft(float complex in[], float complex out[], int n);
static void hglm_fft_internal_(float in[], float complex out[], int n, int stride);
static void hglm_ifft_internal_(float complex in[], float complex out[], int n, int stride);

static HGL_INLINE float hglm_nm_newton_raphson(float (*f)(float), float (*df)(float), float x0);
static HGL_INLINE float hglm_nm_secant(float (*f)(float), float x0, float x1);
static HGL_INLINE float hglm_nm_trapezoidal(float (*f)(float), float a, float b, int N);
static HGL_INLINE float hglm_nm_simpson1(float (*f)(float), float a, float b, int N);
static HGL_INLINE float hglm_nm_euler(float (*f)(float, float), float x0, float y0, float xn, float h);
static HGL_INLINE float hglm_nm_runge_kutta4(float (*f)(float, float), float x0, float y0, float xn, float h);
static HGL_INLINE float hglm_nm_gauss_jordan_solver(HglmMat A, HglmMat x, HglmMat b);

/* ========== HglmIVec2 ======================================================*/

#define hglm_ivec2_print(v) (printf("%s = {%d, %d}\n", #v , (v).x, (v).y))

static HGL_INLINE HglmIVec2 hglm_ivec2(int x, int y)
{
    return (HglmIVec2) {.x = x, .y = y};
}

static HGL_INLINE HglmIVec2 hglm_ivec2_add(HglmIVec2 a, HglmIVec2 b)
{
    return (HglmIVec2) {.x = a.x + b.x, .y = a.y + b.y};
}

static HGL_INLINE HglmIVec2 hglm_ivec2_sub(HglmIVec2 a, HglmIVec2 b)
{
    return (HglmIVec2) {.x = a.x - b.x, .y = a.y - b.y};
}

static HGL_INLINE float hglm_ivec2_distance(HglmIVec2 a, HglmIVec2 b)
{
    int dx = b.x - a.x;
    int dy = b.y - a.y;
    return (int) sqrtf(dx*dx + dy*dy);
}

static HGL_INLINE float hglm_ivec2_length(HglmIVec2 v)
{
    return (int) sqrtf(v.x * v.x + v.y * v.y);
}

static HGL_INLINE HglmIVec2 hglm_ivec2_mul_scalar(HglmIVec2 v, float s)
{
    return (HglmIVec2) {.x = s * v.x, .y = s * v.y};
}

static HGL_INLINE HglmIVec2 hglm_ivec2_lerp(HglmIVec2 a, HglmIVec2 b, float amount)
{
    return hglm_ivec2(
        (int)hglm_lerp(a.x, b.x, amount),
        (int)hglm_lerp(a.y, b.y, amount)
    );
}


/* ========== HglmIVec3 =======================================================*/

#define hglm_ivec3_print(v) (printf("%s = {%d, %d, %d}\n", #v , (v).x, (v).y, (v).z))

static HGL_INLINE HglmIVec3 hglm_ivec3(int x, int y, int z)
{
    return (HglmIVec3) {.x = x, .y = y, .z = z};
}

static HGL_INLINE HglmIVec3 hglm_ivec3_add(HglmIVec3 a, HglmIVec3 b)
{
    return (HglmIVec3) {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}


/* ========== HglmIVec4 =======================================================*/

#define hglm_ivec4_print(v) (printf("%s = {%d, %d, %d, %d}\n", #v , (v).x, (v).y, (v).z, (v).w))

static HGL_INLINE HglmIVec4 hglm_ivec4(int x, int y, int z, int w)
{
    return (HglmIVec4) {.x = x, .y = y, .z = z, .w = w};
}


/* ========== HglmVec2 =======================================================*/

#define hglm_vec2_print(v) (printf("%s = {%f, %f}\n", #v , (double)(v).x, (double)(v).y))

static HGL_INLINE HglmVec2 hglm_vec2(float x, float y)
{
    return (HglmVec2){.x = x, .y = y};
}

static HGL_INLINE HglmVec2 hglm_vec2_from_polar(float r, float phi)
{
    return (HglmVec2) {
        .x = r * cosf(phi),
        .y = r * sinf(phi),
    };
}

static HGL_INLINE HglmVec2 hglm_vec2_add(HglmVec2 a, HglmVec2 b)
{
    return (HglmVec2){.x = a.x + b.x, .y = a.y + b.y};
}

static HGL_INLINE HglmVec2 hglm_vec2_sub(HglmVec2 a, HglmVec2 b)
{
    return (HglmVec2){.x = a.x - b.x, .y = a.y - b.y};
}

static HGL_INLINE float hglm_vec2_distance(HglmVec2 a, HglmVec2 b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx*dx + dy*dy);
}

static HGL_INLINE float hglm_vec2_length(HglmVec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static HGL_INLINE HglmVec2 hglm_vec2_normalize(HglmVec2 v)
{
    float ilen = 1.0f / hglm_vec2_length(v);
    return (HglmVec2) {.x = v.x * ilen, .y = v.y * ilen};
}

static HGL_INLINE float hglm_vec2_dot(HglmVec2 a, HglmVec2 b)
{
    return a.x * b.x + a.y *b.y;
}

static HGL_INLINE float hglm_vec2_wedge(HglmVec2 a, HglmVec2 b)
{
    return (a.x * b.y) - (a.y * b.x);
}

static HGL_INLINE HglmVec2 hglm_vec2_recip(HglmVec2 v)
{
    return (HglmVec2) {.x = 1.0f/v.x, .y = 1.0f/v.y};
}

static HGL_INLINE HglmVec2 hglm_vec2_hadamard(HglmVec2 a, HglmVec2 b)
{
    return (HglmVec2) {.x = a.x * b.x, .y = a.y * b.y};
}

static HGL_INLINE HglmVec2 hglm_vec2_mul_scalar(HglmVec2 v, float s)
{
    return (HglmVec2) {.x = s * v.x, .y = s * v.y};
}

static HGL_INLINE HglmVec2 hglm_vec2_reflect(HglmVec2 v, HglmVec2 normal)
{
    return hglm_vec2_sub(v, hglm_vec2_mul_scalar(normal, 2*hglm_vec2_dot(v, normal)));
}

static HGL_INLINE HglmVec2 hglm_vec2_lerp(HglmVec2 a, HglmVec2 b, float t)
{
    return hglm_vec2_add(
        hglm_vec2_mul_scalar(a, 1.0f - t),
        hglm_vec2_mul_scalar(b, t)
    );
}

static HGL_INLINE HglmVec2 hglm_vec2_slerp(HglmVec2 a, HglmVec2 b, float t)
{
    float omega = acosf(hglm_vec2_dot(a, b));
    return hglm_vec2_add(
        hglm_vec2_mul_scalar(a, sinf((1.0f - t)*omega)/sinf(omega)),
        hglm_vec2_mul_scalar(b, sinf(t*omega)/sinf(omega))
    );
}

static HGL_INLINE HglmVec2 hglm_vec2_bezier3(HglmVec2 v0, HglmVec2 v1, HglmVec2 v2, HglmVec2 v3, float t)
{
    HglmVec4 bezier3 = hglm_bezier3f(t);
    return hglm_vec2_add(
        hglm_vec2_add(
            hglm_vec2_mul_scalar(v0, bezier3.x),
            hglm_vec2_mul_scalar(v1, bezier3.y)
        ),
        hglm_vec2_add(
            hglm_vec2_mul_scalar(v2, bezier3.z),
            hglm_vec2_mul_scalar(v3, bezier3.w)
        )
    );
}


/* ========== HglmVec3 =======================================================*/

#define hglm_vec3_print(v) (printf("%s = {%f, %f, %f}\n", #v , (double)(v).x, (double)(v).y, (double)(v).z))

static HGL_INLINE HglmVec3 hglm_vec3(float x, float y, float z)
{
    return (HglmVec3){.x = x, .y = y, .z = z};
}

static HGL_INLINE HglmVec3 hglm_vec3_from_spherical(float r, float phi, float theta)
{
    return (HglmVec3) {
        .x = r * cosf(theta) * sinf(phi), 
        .y = r * sinf(theta) * sinf(phi), 
        .z = r * cosf(phi),
    };
}

static HGL_INLINE HglmVec3 hglm_vec3_add(HglmVec3 a, HglmVec3 b)
{
    return (HglmVec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

static HGL_INLINE HglmVec3 hglm_vec3_sub(HglmVec3 a, HglmVec3 b)
{
    return (HglmVec3){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

static HGL_INLINE float hglm_vec3_distance(HglmVec3 a, HglmVec3 b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

static HGL_INLINE float hglm_vec3_length(HglmVec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static HGL_INLINE HglmVec3 hglm_vec3_normalize(HglmVec3 v)
{
    float ilen = 1.0f / hglm_vec3_length(v);
    return (HglmVec3) {.x = v.x * ilen, .y = v.y * ilen, .z = v.z * ilen};
}

static HGL_INLINE float hglm_vec3_dot(HglmVec3 a, HglmVec3 b)
{
    return a.x * b.x + a.y *b.y + a.z * b.z;
}

static HGL_INLINE HglmVec3 hglm_vec3_cross(HglmVec3 a, HglmVec3 b)
{
    return (HglmVec3) {
        .x = (a.y * b.z) - (a.z * b.y),
        .y = (a.z * b.x) - (a.x * b.z),
        .z = (a.x * b.y) - (a.y * b.x)
    };
}

static HGL_INLINE HglmVec3 hglm_vec3_recip(HglmVec3 v)
{
    return (HglmVec3) {.x = 1.0f/v.x, .y = 1.0f/v.y, .z = 1.0f/v.z};
}

static HGL_INLINE HglmVec3 hglm_vec3_hadamard(HglmVec3 a, HglmVec3 b)
{
    return (HglmVec3) {.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z};
}

static HGL_INLINE HglmVec3 hglm_vec3_mul_scalar(HglmVec3 v, float s)
{
    return (HglmVec3) {.x = s * v.x, .y = s * v.y, .z = s * v.z};
}

static HGL_INLINE HglmVec3 hglm_vec3_reflect(HglmVec3 v, HglmVec3 normal)
{
    return hglm_vec3_sub(v, hglm_vec3_mul_scalar(normal, 2*hglm_vec3_dot(v, normal)));
}

static HGL_INLINE HglmVec3 hglm_vec3_lerp(HglmVec3 a, HglmVec3 b, float t)
{
    return hglm_vec3_add(
        hglm_vec3_mul_scalar(a, 1.0f - t),
        hglm_vec3_mul_scalar(b, t)
    );
}

static HGL_INLINE HglmVec3 hglm_vec3_slerp(HglmVec3 a, HglmVec3 b, float t)
{
    float omega = acosf(hglm_vec3_dot(a, b));
    return hglm_vec3_add(
        hglm_vec3_mul_scalar(a, sinf((1.0f - t)*omega)/sinf(omega)),
        hglm_vec3_mul_scalar(b, sinf(t*omega)/sinf(omega))
    );
}

static HGL_INLINE HglmVec3 hglm_vec3_bezier3(HglmVec3 v0, HglmVec3 v1, HglmVec3 v2, HglmVec3 v3, float t)
{
    HglmVec4 bezier3 = hglm_bezier3f(t);
    return hglm_vec3_add(
        hglm_vec3_add(
            hglm_vec3_mul_scalar(v0, bezier3.x),
            hglm_vec3_mul_scalar(v1, bezier3.y)
        ),
        hglm_vec3_add(
            hglm_vec3_mul_scalar(v2, bezier3.z),
            hglm_vec3_mul_scalar(v3, bezier3.w)
        )
    );
}


/* ========== HglmVec4 =======================================================*/

#define hglm_vec4_print(v) (printf("%s = {%f, %f, %f, %f}\n", #v , \
                            (double)(v).x, (double)(v).y, (double)(v).z, (double)(v).w))

static HGL_INLINE HglmVec4 hglm_vec4(float x, float y, float z, float w)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4){.v = _mm_set_ps(w, z, y, x)};
#else
    return (HglmVec4){.x = x, .y = y, .z = z, .w = w};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_add(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4){.v = _mm_add_ps(a.v, b.v)};
#else
    return (HglmVec4){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_sub(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4){.v = _mm_sub_ps(a.v, b.v)};
#else
    return (HglmVec4){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z, .w = a.w - b.w};
#endif
}

static HGL_INLINE float hglm_vec4_distance(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    __m128 d = _mm_sub_ps(b.v, a.v);
    d = _mm_mul_ps(d, d);
    d = _mm_hadd_ps(d,d);
    d = _mm_hadd_ps(d,d);
    return sqrtf(_mm_cvtss_f32(d));

    /* SSE1 */
    //__m128 shuf   = _mm_shuffle_ps(d, d, _MM_SHUFFLE(2, 3, 0, 1));  // [ C D | A B ]
    //__m128 sums   = _mm_add_ps(d, shuf);      // sums = [ D+C C+D | B+A A+B ]
    //shuf          = _mm_movehl_ps(shuf, sums);      //  [   C   D | D+C C+D ]  // let the compiler avoid a mov by reusing shuf
    //sums          = _mm_add_ss(sums, shuf);
    //return sqrtf(_mm_cvtss_f32(sums));
#else
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    float dw = b.w - a.w;
    return sqrtf(dx*dx + dy*dy + dz*dz + dw*dw);
#endif
}

static HGL_INLINE float hglm_vec4_length(HglmVec4 v)
{
#ifdef HGLM_USE_SIMD
    __m128 d = _mm_mul_ps(v.v, v.v);
    d = _mm_hadd_ps(d,d);
    d = _mm_hadd_ps(d,d);
    return sqrtf(_mm_cvtss_f32(d));
#else
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_normalize(HglmVec4 v)
{
#ifdef HGLM_USE_SIMD
    float rlen = 1.0f / hglm_vec4_length(v);
    __m128 vrlen = _mm_broadcast_ss(&rlen);
    return (HglmVec4) {.v = _mm_mul_ps(v.v, vrlen)};
#else
    float len = hglm_vec4_length(v);
    return (HglmVec4) {.x = v.x / len, .y = v.y / len, .z = v.z / len, .w = v.w / len};
#endif
}

static HGL_INLINE float hglm_vec4_dot(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    __m128 v = _mm_mul_ps(a.v, b.v);
    v = _mm_hadd_ps(v,v);
    v = _mm_hadd_ps(v,v);
    return _mm_cvtss_f32(v);
#else
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_recip(HglmVec4 v)
{
    return (HglmVec4) {.x = 1.0f/v.x, .y = 1.0f/v.y, .z = 1.0f/v.z, .w = 1.0f/v.w};
}

static HGL_INLINE HglmVec4 hglm_vec4_hadamard(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4) {.v = _mm_mul_ps(a.v, b.v)};
#else
    return (HglmVec4) {.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z, .w = a.w * b.w};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_mul_scalar(HglmVec4 v, float s)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4) {.v = _mm_mul_ps(v.v, _mm_broadcast_ss(&s))};
#else
    return (HglmVec4) {.x = s * v.x, .y = s * v.y, .z = s * v.z, .w = s * v.w};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_swizzle(HglmVec4 v, int a, int b, int c, int d)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4) {.v = _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(d, c, b, a))};
#else
    return (HglmVec4) {.x = v.f[a], .y = v.f[b], .z = v.f[c], .w = v.f[d]};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_perspective_divide(HglmVec4 v)
{
    HglmVec4 u;
    u.x = v.x / v.w;
    u.y = v.y / v.w;
    u.z = v.z / v.w;
    u.w = 1.0f;
    return u;
}

static HGL_INLINE HglmVec4 hglm_vec4_lerp(HglmVec4 a, HglmVec4 b, float t)
{
    return hglm_vec4_add(
        hglm_vec4_mul_scalar(a, 1.0f - t),
        hglm_vec4_mul_scalar(b, t)
    );
}

static HGL_INLINE HglmVec4 hglm_vec4_bezier3(HglmVec4 v0, HglmVec4 v1, HglmVec4 v2, HglmVec4 v3, float t)
{
    HglmVec4 bezier3 = hglm_bezier3f(t);
    return hglm_vec4_add(
        hglm_vec4_add(
            hglm_vec4_mul_scalar(v0, bezier3.x),
            hglm_vec4_mul_scalar(v1, bezier3.y)
        ),
        hglm_vec4_add(
            hglm_vec4_mul_scalar(v2, bezier3.z),
            hglm_vec4_mul_scalar(v3, bezier3.w)
        )
    );
}


/* ========== HglmQuat =======================================================*/

static HGL_INLINE HglmQuat hglm_quat(float w, float x, float y, float z)
{
    return (HglmQuat) {
        .x = x,
        .y = y,
        .z = z,
        .w = w,
    };
}

static HGL_INLINE HglmQuat hglm_quat_from_angle_axis(float angle, HglmVec3 axis)
{
    float s = sinf(angle / 2.0f);
    return (HglmQuat) {
        .x = s * axis.x,
        .y = s * axis.y,
        .z = s * axis.z,
        .w = cosf(angle / 2.0f),
    };
}

static HGL_INLINE HglmQuat hglm_quat_mul_quat(HglmQuat a, HglmQuat b)
{
    return (HglmQuat) {
        .x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        .y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        .z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        .w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
    };
}

static HGL_INLINE HglmQuat hglm_quat_mul_scalar(HglmQuat q, float s)
{
    return (HglmQuat) {
        .x = q.x * s,
        .y = q.y * s,
        .z = q.z * s,
        .w = q.w * s,
    };
}

static HGL_INLINE HglmQuat hglm_quat_pow(HglmQuat q, float y)
{
    const float EPSILON = 0.00001f;
    float ql = hglm_quat_length(q);
    if (ql < EPSILON) {
        return (HglmQuat){.x = 0, .y = 0, .z = 0, .w = 0};
    }

    float lp = powf(ql, y);
    float theta = acosf(q.w / ql);
    float w = lp * cosf(y * theta);

    float vl = hglm_vec3_length(q.xyz);
    if (vl < EPSILON) {
        return (HglmQuat){.x = 0, .y = 0, .z = 0, .w = w};
    }

    float s = lp * sinf(y * theta) / vl;
    return (HglmQuat) {
        .x = s * q.x,
        .y = s * q.y,
        .z = s * q.z,
        .w = w,
    };
}

static HGL_INLINE float hglm_quat_length(HglmQuat q)
{
    return sqrtf(hglm_quat_square_length(q));
}

static HGL_INLINE float hglm_quat_square_length(HglmQuat q)
{
    return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

static HGL_INLINE HglmQuat hglm_quat_conjugate(HglmQuat q)
{
    return (HglmQuat) {
        .x = -q.x,
        .y = -q.y,
        .z = -q.z,
        .w =  q.w,
    };
}

static HGL_INLINE HglmQuat hglm_quat_inverse(HglmQuat q)
{
    float r_sqare_len = 1.0f / hglm_quat_square_length(q);
    HglmQuat conjugate = hglm_quat_conjugate(q);
    return hglm_quat_mul_scalar(conjugate, r_sqare_len);
}

static HGL_INLINE HglmVec3 hglm_quat_apply_vec3(HglmQuat q, HglmVec3 v)
{
    HglmQuat p = {
        .xyz = v,
        .w   = 0.0f,
    };
    HglmQuat q_inv = hglm_quat_inverse(q);
    return hglm_quat_mul_quat(hglm_quat_mul_quat(q, p), q_inv).xyz;
}

static HGL_INLINE HglmQuat hglm_quat_slerp(HglmQuat q0, HglmQuat q1, float t, bool shortest_path)
{
    if (shortest_path && (hglm_vec4_dot(q0, q1) < 0.0f)) {
        q1 = (HglmQuat) {.x = q1.x, .y = q1.y, .z = q1.z, .w = q1.w};
    }
    HglmQuat q0_inv = hglm_quat_inverse(q0);
    HglmQuat q1q0_inv = hglm_quat_mul_quat(q1, q0_inv);
    HglmQuat q1q0_inv_t = hglm_quat_pow(q1q0_inv, t);
    return hglm_quat_mul_quat(q1q0_inv_t, q0);
}


/* ========== HglmMat3 =======================================================*/

#define hglm_mat2_print(m)                        \
(                                                 \
    printf("%s = \n"                              \
           "    |%14.5f %14.5f |\n"               \
           "    |%14.5f %14.5f |\n", #m ,         \
            (double) (m).c0.x, (double) (m).c1.x, \
            (double) (m).c0.y, (double) (m).c1.y) \
)

static HGL_INLINE HglmMat2 hglm_mat2(HglmVec2 c0, HglmVec2 c1)
{
    return (HglmMat2){.c0 = c0, .c1 = c1};
}

static HGL_INLINE HglmMat2 hglm_mat2_identity()
{
    return HGLM_MAT2_IDENTITY;
}

static HGL_INLINE HglmMat2 hglm_mat2_make_scale(HglmVec2 v)
{
    HglmMat2 s = HGLM_MAT2_IDENTITY;
    s.c0.x = v.x;
    s.c1.y = v.y;
    return s;
}

static HGL_INLINE HglmMat2 hglm_mat2_make_rotation(float angle)
{
    float O = angle;
    float c0x =  cos(O);
    float c1x = -sin(O);
    float c0y =  sin(O);
    float c1y =  cos(O);
    return (HglmMat2){
        .c0 = {.x = c0x, .y = c0y},
        .c1 = {.x = c1x, .y = c1y},
    };
}

static HGL_INLINE HglmMat2 hglm_mat2_adjugate(HglmMat2 m)
{
    float a, b, c, d;
    a = m.m00; b = m.m01;
    c = m.m10; d = m.m11;
    return (HglmMat2){
        .c0 = {.x =  d, .y = -c},
        .c1 = {.x = -b, .y =  a},
    };
}

static HGL_INLINE HglmMat2 hglm_mat2_inverse(HglmMat2 m)
{
    float r_det = 1.0f / hglm_mat2_determinant(m);
    return hglm_mat2_mul_scalar(hglm_mat2_adjugate(m), r_det); 
}

static HGL_INLINE HglmMat2 hglm_mat2_add(HglmMat2 a, HglmMat2 b)
{
    return (HglmMat2) {
        .c0 = hglm_vec2_add(a.c0, b.c0),
        .c1 = hglm_vec2_add(a.c1, b.c1),
    };
}

static HGL_INLINE HglmMat2 hglm_mat2_sub(HglmMat2 a, HglmMat2 b)
{
    return (HglmMat2) {
        .c0 = hglm_vec2_sub(a.c0, b.c0),
        .c1 = hglm_vec2_sub(a.c1, b.c1),
    };
}


static HGL_INLINE HglmMat2 hglm_mat2_mul_scalar(HglmMat2 m, float s)
{
    return (HglmMat2) {
        .c0 = {.x = s * m.c0.x, .y = s * m.c0.y},
        .c1 = {.x = s * m.c1.x, .y = s * m.c1.y},
    };
}

static HGL_INLINE HglmVec2 hglm_mat2_mul_vec2(HglmMat2 m, HglmVec2 v)
{
    return (HglmVec2) {
        .x = m.c0.x * v.x + m.c1.x * v.y,
        .y = m.c0.y * v.x + m.c1.y * v.y,
    };
}

static HGL_INLINE HglmMat2 hglm_mat2_mul_mat2(HglmMat2 a, HglmMat2 b)
{
    return (HglmMat2) {
        /* c0 */
        .m00 = a.c0.x * b.c0.x + a.c1.x * b.c0.y, 
        .m10 = a.c0.y * b.c0.x + a.c1.y * b.c0.y, 
        /* c1 */
        .m01 = a.c0.x * b.c1.x + a.c1.x * b.c1.y, 
        .m11 = a.c0.y * b.c1.x + a.c1.y * b.c1.y, 
    };
}

static HGL_INLINE HglmMat2 hglm_mat2_scale(HglmMat2 m, HglmVec2 v)
{
    return hglm_mat2_mul_mat2(m, hglm_mat2_make_scale(v));
}

static HGL_INLINE HglmMat2 hglm_mat2_rotate(HglmMat2 m, float angle)
{
    return hglm_mat2_mul_mat2(m, hglm_mat2_make_rotation(angle));
}

static HGL_INLINE float hglm_mat2_determinant(HglmMat2 m)
{
    float a, b, c, d;
    a = m.m00; b = m.m01;
    c = m.m10; d = m.m11;
    return a*d - b*c;
}



/* ========== HglmMat3 =======================================================*/

#define hglm_mat3_print(m)                                           \
(                                                                    \
    printf("%s = \n"                                                 \
           "    |%14.5f %14.5f %14.5f |\n"                           \
           "    |%14.5f %14.5f %14.5f |\n"                           \
           "    |%14.5f %14.5f %14.5f |\n", #m ,                     \
            (double) (m).c0.x, (double) (m).c1.x, (double) (m).c2.x, \
            (double) (m).c0.y, (double) (m).c1.y, (double) (m).c2.y, \
            (double) (m).c0.z, (double) (m).c1.z, (double) (m).c2.z) \
)

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3(HglmVec3 c0, HglmVec3 c1, HglmVec3 c2)
{
    return (HglmMat3){.c0 = c0, .c1 = c1, .c2 = c2};
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_identity(void)
{
    return HGLM_MAT3_IDENTITY;
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_scale(HglmVec3 v)
{
    HglmMat3 s = HGLM_MAT3_IDENTITY;
    s.c0.x = v.x;
    s.c1.y = v.y;
    s.c2.z = v.z;
    return s;
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_rotation(float angle, HglmVec3 axis)
{
    float O = angle;
    float ux = axis.x;
    float uy = axis.y;
    float uz = axis.z;
    float c0x = cosf(O) + ux*ux * (1 - cosf(O));
    float c1x = ux*uy * (1 - cosf(O)) - uz * sinf(O);
    float c2x = ux*uz * (1 - cosf(O)) + uy * sinf(O);
    float c0y = uy*ux * (1 - cosf(O)) + uz * sinf(O);
    float c1y = cosf(O) + uy*uy * (1 - cosf(O));
    float c2y = uy*uz * (1 - cosf(O)) - ux * sinf(O);
    float c0z = uz*ux * (1 - cosf(O)) - uy * sinf(O);
    float c1z = uz*uy * (1 - cosf(O)) + ux * sinf(O);
    float c2z = cosf(O) + uz*uz * (1 - cosf(O));
    return (HglmMat3) {
        .c0 = {.x =  c0x, .y =  c0y, .z =  c0z},
        .c1 = {.x =  c1x, .y =  c1y, .z =  c1z},
        .c2 = {.x =  c2x, .y =  c2y, .z =  c2z},
    };
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_translation(HglmVec2 v)
{
    HglmMat3 t = HGLM_MAT3_IDENTITY;
    t.c2.xy = v;
    return t;
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_demote_from_mat4(HglmMat4 mat4)
{
    return (HglmMat3){.c0 = mat4.c0.xyz, .c1 = mat4.c1.xyz, .c2 = mat4.c2.xyz};
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_adjugate(HglmMat3 m)
{
    float a1, a2, a3, b1, b2, b3, c1, c2, c3;
    a1 = m.m00; a2 = m.m01; a3 = m.m02;
    b1 = m.m10; b2 = m.m11; b3 = m.m12;
    c1 = m.m20; c2 = m.m21; c3 = m.m22;
    HglmMat3 C = (HglmMat3) {
        .m00 =  hglm_mat2_determinant((HglmMat2){.m00 = b2, .m10 = c2, .m01 = b3, .m11 = c3}),
        .m01 = -hglm_mat2_determinant((HglmMat2){.m00 = b1, .m10 = c1, .m01 = b3, .m11 = c3}),
        .m02 =  hglm_mat2_determinant((HglmMat2){.m00 = b1, .m10 = c1, .m01 = b2, .m11 = c2}),
        .m10 = -hglm_mat2_determinant((HglmMat2){.m00 = a2, .m10 = c2, .m01 = a3, .m11 = c3}),
        .m11 =  hglm_mat2_determinant((HglmMat2){.m00 = a1, .m10 = c1, .m01 = a3, .m11 = c3}),
        .m12 = -hglm_mat2_determinant((HglmMat2){.m00 = a1, .m10 = c1, .m01 = a2, .m11 = c2}),
        .m20 =  hglm_mat2_determinant((HglmMat2){.m00 = a2, .m10 = b2, .m01 = a3, .m11 = b3}),
        .m21 = -hglm_mat2_determinant((HglmMat2){.m00 = a1, .m10 = b1, .m01 = a3, .m11 = b3}),
        .m22 =  hglm_mat2_determinant((HglmMat2){.m00 = a1, .m10 = b1, .m01 = a2, .m11 = b2}),
    };
    return hglm_mat3_transpose(C);
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_inverse(HglmMat3 m)
{
    float r_det = 1.0f / hglm_mat3_determinant(m);
    return hglm_mat3_mul_scalar(hglm_mat3_adjugate(m), r_det); 
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_affine_inverse(HglmMat3 m)
{
    HglmMat2 L = (HglmMat2){.c0 = m.c0.xy, .c1 = m.c1.xy};
    HglmVec2 t = m.c2.xy;
    HglmMat2 L_inv = hglm_mat2_inverse(L);
    HglmMat3 M = hglm_mat3_identity();
    M.c0.xy = L_inv.c0;
    M.c1.xy = L_inv.c1;
    M.c2.xy = hglm_vec2_mul_scalar(hglm_mat2_mul_vec2(L_inv, t), -1.0f);
    return M;
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_transpose(HglmMat3 m)
{
    return (HglmMat3) {
        .c0 = {.x = m.c0.x, .y = m.c1.x, .z = m.c2.x},
        .c1 = {.x = m.c0.y, .y = m.c1.y, .z = m.c2.y},
        .c2 = {.x = m.c0.z, .y = m.c1.z, .z = m.c2.z},
    };
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_add(HglmMat3 a, HglmMat3 b)
{
    return (HglmMat3) {
        .c0 = hglm_vec3_add(a.c0, b.c0),
        .c1 = hglm_vec3_add(a.c1, b.c1),
        .c2 = hglm_vec3_add(a.c2, b.c2),
    };
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_sub(HglmMat3 a, HglmMat3 b)
{
    return (HglmMat3) {
        .c0 = hglm_vec3_sub(a.c0, b.c0),
        .c1 = hglm_vec3_sub(a.c1, b.c1),
        .c2 = hglm_vec3_sub(a.c2, b.c2),
    };
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_mul_scalar(HglmMat3 m, float s)
{
    return (HglmMat3) {
        .c0 = {.x = s * m.c0.x, .y = s * m.c0.y, .z = s * m.c0.z},
        .c1 = {.x = s * m.c1.x, .y = s * m.c1.y, .z = s * m.c1.z},
        .c2 = {.x = s * m.c2.x, .y = s * m.c2.y, .z = s * m.c2.z},
    };
}

__attribute__ ((const, unused)) static HGL_INLINE HglmVec3 hglm_mat3_mul_vec3(HglmMat3 m, HglmVec3 v)
{
    return (HglmVec3) {
        .x = m.c0.x * v.x + m.c1.x * v.y + m.c2.x * v.z,
        .y = m.c0.y * v.x + m.c1.y * v.y + m.c2.y * v.z,
        .z = m.c0.z * v.x + m.c1.z * v.y + m.c2.z * v.z,
    };
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_mul_mat3(HglmMat3 a, HglmMat3 b)
{
    return (HglmMat3) {
        /* c0 */
        .m00 = a.c0.x * b.c0.x + a.c1.x * b.c0.y + a.c2.x * b.c0.z,
        .m10 = a.c0.y * b.c0.x + a.c1.y * b.c0.y + a.c2.y * b.c0.z,
        .m20 = a.c0.z * b.c0.x + a.c1.z * b.c0.y + a.c2.z * b.c0.z,
        /* c1 */
        .m01 = a.c0.x * b.c1.x + a.c1.x * b.c1.y + a.c2.x * b.c1.z,
        .m11 = a.c0.y * b.c1.x + a.c1.y * b.c1.y + a.c2.y * b.c1.z,
        .m21 = a.c0.z * b.c1.x + a.c1.z * b.c1.y + a.c2.z * b.c1.z,
        /* c2 */
        .m02 = a.c0.x * b.c2.x + a.c1.x * b.c2.y + a.c2.x * b.c2.z,
        .m12 = a.c0.y * b.c2.x + a.c1.y * b.c2.y + a.c2.y * b.c2.z,
        .m22 = a.c0.z * b.c2.x + a.c1.z * b.c2.y + a.c2.z * b.c2.z,
    };
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_scale(HglmMat3 m, HglmVec3 v)
{
    return hglm_mat3_mul_mat3(m, hglm_mat3_make_scale(v));
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_rotate(HglmMat3 m, float angle, HglmVec3 axis)
{
    return hglm_mat3_mul_mat3(m, hglm_mat3_make_rotation(angle, axis));
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_translate(HglmMat3 m, HglmVec2 v)
{
    return hglm_mat3_mul_mat3(m, hglm_mat3_make_translation(v));
}

__attribute__ ((const, unused)) static HGL_INLINE float hglm_mat3_determinant(HglmMat3 m)
{
    float a, b, c, d, e, f, g, h, i;
    a = m.m00; b = m.m01; c = m.m02;
    d = m.m10; e = m.m11; f = m.m12;
    g = m.m20; h = m.m21; i = m.m22;
    return a*e*i + b*f*g + c*d*h - c*e*g - b*d*i - a*f*h;
}


/* ========== HglmMat4 =======================================================*/

#define hglm_mat4_print(m)                                                              \
(                                                                                       \
    printf("%s = \n"                                                                    \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
           "    |%14.5f %14.5f %14.5f %14.5f |\n", #m ,                                 \
            (double) (m).c0.x, (double) (m).c1.x, (double) (m).c2.x, (double) (m).c3.x, \
            (double) (m).c0.y, (double) (m).c1.y, (double) (m).c2.y, (double) (m).c3.y, \
            (double) (m).c0.z, (double) (m).c1.z, (double) (m).c2.z, (double) (m).c3.z, \
            (double) (m).c0.w, (double) (m).c1.w, (double) (m).c2.w, (double) (m).c3.w) \
)

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4(HglmVec4 c0,
                                     HglmVec4 c1,
                                     HglmVec4 c2,
                                     HglmVec4 c3)
{
    return (HglmMat4){.c0 = c0, .c1 = c1, .c2 = c2, .c3 = c3};
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_zero()
{
    return (HglmMat4){0};
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_identity(void)
{
    return HGLM_MAT4_IDENTITY;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_scale(HglmVec3 v)
{
    HglmMat4 s = HGLM_MAT4_IDENTITY;
    s.c0.x = v.x;
    s.c1.y = v.y;
    s.c2.z = v.z;
    return s;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_rotation(float angle, HglmVec3 axis)
{
    float O = angle;
    float ux = axis.x;
    float uy = axis.y;
    float uz = axis.z;
    float c0x = cosf(O) + ux*ux * (1 - cosf(O));
    float c1x = ux*uy * (1 - cosf(O)) - uz * sinf(O);
    float c2x = ux*uz * (1 - cosf(O)) + uy * sinf(O);
    float c0y = uy*ux * (1 - cosf(O)) + uz * sinf(O);
    float c1y = cosf(O) + uy*uy * (1 - cosf(O));
    float c2y = uy*uz * (1 - cosf(O)) - ux * sinf(O);
    float c0z = uz*ux * (1 - cosf(O)) - uy * sinf(O);
    float c1z = uz*uy * (1 - cosf(O)) + ux * sinf(O);
    float c2z = cosf(O) + uz*uz * (1 - cosf(O));
    return (HglmMat4) {
        .c0 = {.x =  c0x, .y =  c0y, .z =  c0z, .w = 0.0f},
        .c1 = {.x =  c1x, .y =  c1y, .z =  c1z, .w = 0.0f},
        .c2 = {.x =  c2x, .y =  c2y, .z =  c2z, .w = 0.0f},
        .c3 = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f},
    };
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_translation(HglmVec3 v)
{
    HglmMat4 t = HGLM_MAT4_IDENTITY;
    t.c3.xyz = v;
    return t;
}

__attribute__ ((const, unused)) 
static HGL_INLINE HglmMat4 hglm_mat4_promote_from_mat3(HglmMat3 mat3)
{
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.xyz = mat3.c0;
    m.c1.xyz = mat3.c1;
    m.c2.xyz = mat3.c2;
    return m;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_ortho(float left, float right, float bottom,
                                           float top,  float near,  float far)
{
#if 0
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x = 2 / (right - left);
    m.c1.y = 2 / (top - bottom);
    m.c2.z = -1 / (far - near);
    m.c3.x = -((left + right) / (right - left));
    m.c3.y = -((bottom + top) / (top - bottom));
    m.c3.z = -((near)   / (far - near));
#else
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x =  2 / (right - left);
    m.c1.y =  2 / (top - bottom);
    m.c2.z = -2 / (far - near); // Note: inversion
    m.c3.x = -((right + left) / (right - left));
    m.c3.y = -((top + bottom) / (top - bottom));
    m.c3.z = -((far + near)   / (far - near));
#endif
    return m;
}

__attribute__ ((const, unused)) 
static HGL_INLINE HglmMat4 hglm_mat4_ortho_inverse(float left, float right, float bottom,
                                                   float top,  float near,  float far)
{
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x = (right - left) / 2.0f;
    m.c1.y = (top - bottom) / 2.0f;
    m.c2.z = -((far - near) / 2.0f); // Note: inversion
    m.c3.x = (right + left) / 2.0f;
    m.c3.y = (top + bottom) / 2.0f;
    m.c3.z = -((far + near) / 2.0f);
    return m;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_perspective(float fov, float aspect, float znear, float zfar)
{
#if 1
    /* x: [-1,1], y: [-1,1], z = [0, 1] */
    float a = 1.0f / aspect;
    float f = 1.0f / tanf(fov/2);
    float d0 = -(zfar + znear) / (zfar - znear);
    float d1 = -(2 * zfar * znear) / (zfar - znear);
    return (HglmMat4) {
        .m00 =   a*f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
        .m10 =  0.0f, .m11 =     f, .m12 =  0.0f, .m13 =  0.0f,
        .m20 =  0.0f, .m21 =  0.0f, .m22 =    d0, .m23 =    d1,
        .m30 =  0.0f, .m31 =  0.0f, .m32 = -1.0f, .m33 =  0.0f,
    };
#else
    /* x: [-1,1], y: [-1,1], z = [-1, 1] */
    float a = 1.0f / aspect;
    float f = 1.0f / tanf(fov/2);
    float d0 = (-znear - zfar) / (znear - zfar);
    float d1 = 2*(-(2 * zfar * znear) / (znear - zfar)) - 1;
    return (HglmMat4) {
        .m00 =   a*f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
        .m10 =  0.0f, .m11 =     f, .m12 =  0.0f, .m13 =  0.0f,
        .m20 =  0.0f, .m21 =  0.0f, .m22 =    d0, .m23 =    d1,
        .m30 =  0.0f, .m31 =  0.0f, .m32 =  1.0f, .m33 =  0.0f,
    };
#endif
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_affine_inverse(HglmMat4 m)
{
    HglmMat3 L = (HglmMat3){.c0 = m.c0.xyz, .c1 = m.c1.xyz, .c2 = m.c2.xyz};
    HglmVec3 t = m.c3.xyz;
    HglmMat3 L_inv = hglm_mat3_inverse(L);
    HglmMat4 M = hglm_mat4_identity();
    M.c0.xyz = L_inv.c0;
    M.c1.xyz = L_inv.c1;
    M.c2.xyz = L_inv.c2;
    M.c3.xyz = hglm_vec3_mul_scalar(hglm_mat3_mul_vec3(L_inv, t), -1.0f);
    return M;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_transpose(HglmMat4 m)
{
#ifdef HGLM_USE_SIMD
    _MM_TRANSPOSE4_PS(m.c0.v, m.c1.v, m.c2.v, m.c3.v);
    return m;
#else
    return (HglmMat4) {
        .c0 = {.x = m.c0.x, .y = m.c1.x, .z = m.c2.x, .w = m.c3.x},
        .c1 = {.x = m.c0.y, .y = m.c1.y, .z = m.c2.y, .w = m.c3.y},
        .c2 = {.x = m.c0.z, .y = m.c1.z, .z = m.c2.z, .w = m.c3.z},
        .c3 = {.x = m.c0.w, .y = m.c1.w, .z = m.c2.w, .w = m.c3.w},
    };
#endif
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_look_at(HglmVec3 camera, HglmVec3 target, HglmVec3 up)
{
    HglmVec3 f = hglm_vec3_normalize(hglm_vec3_sub(target, camera));
    HglmVec3 u = hglm_vec3_normalize(up);
    HglmVec3 s = hglm_vec3_normalize(hglm_vec3_cross(f, u));
    u = hglm_vec3_cross(s, f);
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x =  s.x;
    m.c1.x =  s.y;
    m.c2.x =  s.z;
    m.c0.y =  u.x;
    m.c1.y =  u.y;
    m.c2.y =  u.z;
    m.c0.z = -f.x;
    m.c1.z = -f.y;
    m.c2.z = -f.z;
    m.c3.x = -hglm_vec3_dot(s, camera);
    m.c3.y = -hglm_vec3_dot(u, camera);
    m.c3.z = hglm_vec3_dot(f, camera); // this is a little odd..
    return m;
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_look_to(HglmVec3 camera, HglmVec3 dir, HglmVec3 up)
{
    HglmVec3 f = dir;
    HglmVec3 u = hglm_vec3_normalize(up);
    HglmVec3 s = hglm_vec3_normalize(hglm_vec3_cross(f, u));
    u = hglm_vec3_cross(s, f);
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x =  s.x;
    m.c1.x =  s.y;
    m.c2.x =  s.z;
    m.c0.y =  u.x;
    m.c1.y =  u.y;
    m.c2.y =  u.z;
    m.c0.z = -f.x;
    m.c1.z = -f.y;
    m.c2.z = -f.z;
    m.c3.x = -hglm_vec3_dot(s, camera);
    m.c3.y = -hglm_vec3_dot(u, camera);
    m.c3.z = hglm_vec3_dot(f, camera); // this is a little odd..
    return m;
}


__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_add(HglmMat4 a, HglmMat4 b)
{
#ifdef HGLM_USE_SIMD
    HglmMat4 m;
    m.c0.v = _mm_add_ps(a.c0.v, b.c0.v);
    m.c1.v = _mm_add_ps(a.c1.v, b.c1.v);
    m.c2.v = _mm_add_ps(a.c2.v, b.c2.v);
    m.c3.v = _mm_add_ps(a.c3.v, b.c3.v);
    return m;
#else
    return (HglmMat4) {
        .c0 = hglm_vec4_add(a.c0, b.c0),
        .c1 = hglm_vec4_add(a.c1, b.c1),
        .c2 = hglm_vec4_add(a.c2, b.c2),
        .c3 = hglm_vec4_add(a.c3, b.c3)
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_sub(HglmMat4 a, HglmMat4 b)
{
#ifdef HGLM_USE_SIMD
    HglmMat4 m;
    m.c0.v = _mm_sub_ps(a.c0.v, b.c0.v);
    m.c1.v = _mm_sub_ps(a.c1.v, b.c1.v);
    m.c2.v = _mm_sub_ps(a.c2.v, b.c2.v);
    m.c3.v = _mm_sub_ps(a.c3.v, b.c3.v);
    return m;
#else
    return (HglmMat4) {
        .c0 = hglm_vec4_sub(a.c0, b.c0),
        .c1 = hglm_vec4_sub(a.c1, b.c1),
        .c2 = hglm_vec4_sub(a.c2, b.c2),
        .c3 = hglm_vec4_sub(a.c3, b.c3)
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_mul_scalar(HglmMat4 m, float s)
{
#ifdef HGLM_USE_SIMD
    __m128 vec_s = _mm_broadcast_ss(&s);
    return (HglmMat4) {
        .c0 = {.v = _mm_mul_ps(vec_s, m.c0.v)},
        .c1 = {.v = _mm_mul_ps(vec_s, m.c1.v)},
        .c2 = {.v = _mm_mul_ps(vec_s, m.c2.v)},
        .c3 = {.v = _mm_mul_ps(vec_s, m.c3.v)}
    };
#else
    return (HglmMat4) {
        .c0 = {.x = s * m.c0.x, .y = s * m.c0.y, .z = s * m.c0.z, .w = s * m.c0.w}, // c0
        .c1 = {.x = s * m.c1.x, .y = s * m.c1.y, .z = s * m.c1.z, .w = s * m.c1.w}, // c1
        .c2 = {.x = s * m.c2.x, .y = s * m.c2.y, .z = s * m.c2.z, .w = s * m.c2.w}, // c2
        .c3 = {.x = s * m.c3.x, .y = s * m.c3.y, .z = s * m.c3.z, .w = s * m.c3.w}  // c3
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmVec4 hglm_mat4_mul_vec4(HglmMat4 m, HglmVec4 v)
{
#ifdef HGLM_USE_SIMD
    //(void) m;
    //(void) v;
    //return hglm_vec4(0,0,0,0);

    //__m128 vec_s = _mm_set_ps1(s);



    HglmVec4 res;
    __m128 t0 = _mm_mul_ps(_mm_broadcast_ss(&v.x), m.c0.v);
    __m128 t1 = _mm_mul_ps(_mm_broadcast_ss(&v.y), m.c1.v);
    __m128 t2 = _mm_mul_ps(_mm_broadcast_ss(&v.z), m.c2.v);
    __m128 t3 = _mm_mul_ps(_mm_broadcast_ss(&v.w), m.c3.v);
    res.v = _mm_add_ps(_mm_add_ps(t0, t1),
                       _mm_add_ps(t2, t3));
    return res;

    // hmmmm
    //__m128 r = _mm_mul_ps(_mm_set1_ps(v.x), m.c0.v);
    //r = _mm_fmadd_ps(_mm_set1_ps(v.y), m.c1.v, r);
    //r = _mm_fmadd_ps(_mm_set1_ps(v.z), m.c2.v, r);
    //r = _mm_fmadd_ps(_mm_set1_ps(v.w), m.c3.v, r);
    //return (HglmVec4) {.v = r};

#else
    return (HglmVec4) {
        .x = m.c0.x * v.x + m.c1.x * v.y + m.c2.x * v.z + m.c3.x * v.w,
        .y = m.c0.y * v.x + m.c1.y * v.y + m.c2.y * v.z + m.c3.y * v.w,
        .z = m.c0.z * v.x + m.c1.z * v.y + m.c2.z * v.z + m.c3.z * v.w,
        .w = m.c0.w * v.x + m.c1.w * v.y + m.c2.w * v.z + m.c3.w * v.w,
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_mul_mat4(HglmMat4 a, HglmMat4 b)
{
#ifdef HGLM_USE_SIMD
    //HglmMat4 res;
    //__m128 row[4], sum[4];
    //for (int i = 0; i < 4; i++) row[i] = _mm_load_ps(&a.f[4*i]);
    //for (int i = 0; i < 4; i++) {
    //    sum[i] = _mm_setzero_ps();
    //    for (int j = 0; j < 4; j++) {
    //        sum[i] = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(b.f[i*4 + j]), row[j]), sum[i]);
    //    }
    //}
    //_mm_store_ps((float *)&res.c0.v, sum[0]);
    //_mm_store_ps((float *)&res.c1.v, sum[1]);
    //_mm_store_ps((float *)&res.c2.v, sum[2]);
    //_mm_store_ps((float *)&res.c3.v, sum[3]);
    //return res;


    //HglmMat4 m;
    //__m256 t0, t1, t2;

    //__m256 c01 = _mm256_load_ps((float *)&a.c0);
    //__m256 c23 = _mm256_load_ps((float *)&a.c2);

    //t0     = _mm256_mul_ps(c01, _mm256_set_m128(_mm_broadcast_ss(&b.c0.y), _mm_broadcast_ss(&b.c0.x)));
    //t1     = _mm256_mul_ps(c23, _mm256_set_m128(_mm_broadcast_ss(&b.c0.w), _mm_broadcast_ss(&b.c0.z)));
    //t2     = _mm256_add_ps(t0, t1);
    //m.c0.v = _mm_add_ps(_mm256_castps256_ps128(t2), _mm256_extractf128_ps(t2, 1));

    //t0     = _mm256_mul_ps(c01, _mm256_set_m128(_mm_broadcast_ss(&b.c1.y), _mm_broadcast_ss(&b.c1.x)));
    //t1     = _mm256_mul_ps(c23, _mm256_set_m128(_mm_broadcast_ss(&b.c1.w), _mm_broadcast_ss(&b.c1.z)));
    //t2     = _mm256_add_ps(t0, t1);
    //m.c1.v = _mm_add_ps(_mm256_castps256_ps128(t2), _mm256_extractf128_ps(t2, 1));

    //t0     = _mm256_mul_ps(c01, _mm256_set_m128(_mm_broadcast_ss(&b.c2.y), _mm_broadcast_ss(&b.c2.x)));
    //t1     = _mm256_mul_ps(c23, _mm256_set_m128(_mm_broadcast_ss(&b.c2.w), _mm_broadcast_ss(&b.c2.z)));
    //t2     = _mm256_add_ps(t0, t1);
    //m.c2.v = _mm_add_ps(_mm256_castps256_ps128(t2), _mm256_extractf128_ps(t2, 1));

    //t0     = _mm256_mul_ps(c01, _mm256_set_m128(_mm_broadcast_ss(&b.c3.y), _mm_broadcast_ss(&b.c3.x)));
    //t1     = _mm256_mul_ps(c23, _mm256_set_m128(_mm_broadcast_ss(&b.c3.w), _mm_broadcast_ss(&b.c3.z)));
    //t2     = _mm256_add_ps(t0, t1);
    //m.c3.v = _mm_add_ps(_mm256_castps256_ps128(t2), _mm256_extractf128_ps(t2, 1));

    //return m;

    //HglmMat4 m;
    //__m256 t0, t1, t2;

    //__m256 c01 = _mm256_load_ps((float *)&a.c0);
    //__m256 c23 = _mm256_load_ps((float *)&a.c2);

    //__m256 n01;
    //n01 = _mm256_mul_ps();

    //return m;

    HglmMat4 m;

    __m128 t0, t1, t2, t3;
    __m128 c0, c1, c2, c3;

    /* c0 */
    t0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c0.x));
    t1 = _mm_mul_ps(a.c1.v, _mm_broadcast_ss(&b.c0.y));
    t2 = _mm_mul_ps(a.c2.v, _mm_broadcast_ss(&b.c0.z));
    t3 = _mm_mul_ps(a.c3.v, _mm_broadcast_ss(&b.c0.w));
    c0 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c1 */
    t0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c1.x));
    t1 = _mm_mul_ps(a.c1.v, _mm_broadcast_ss(&b.c1.y));
    t2 = _mm_mul_ps(a.c2.v, _mm_broadcast_ss(&b.c1.z));
    t3 = _mm_mul_ps(a.c3.v, _mm_broadcast_ss(&b.c1.w));
    c1 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c2 */
    t0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c2.x));
    t1 = _mm_mul_ps(a.c1.v, _mm_broadcast_ss(&b.c2.y));
    t2 = _mm_mul_ps(a.c2.v, _mm_broadcast_ss(&b.c2.z));
    t3 = _mm_mul_ps(a.c3.v, _mm_broadcast_ss(&b.c2.w));
    c2 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c3 */
    t0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c3.x));
    t1 = _mm_mul_ps(a.c1.v, _mm_broadcast_ss(&b.c3.y));
    t2 = _mm_mul_ps(a.c2.v, _mm_broadcast_ss(&b.c3.z));
    t3 = _mm_mul_ps(a.c3.v, _mm_broadcast_ss(&b.c3.w));
    c3 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    //c0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c0.x));
    //c0 = _mm_fmadd_ps(a.c1.v, _mm_broadcast_ss(&b.c0.y), c0);
    //c0 = _mm_fmadd_ps(a.c2.v, _mm_broadcast_ss(&b.c0.z), c0);
    //c0 = _mm_fmadd_ps(a.c3.v, _mm_broadcast_ss(&b.c0.w), c0);

    //c1 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c1.x));
    //c1 = _mm_fmadd_ps(a.c1.v, _mm_broadcast_ss(&b.c1.y), c1);
    //c1 = _mm_fmadd_ps(a.c2.v, _mm_broadcast_ss(&b.c1.z), c1);
    //c1 = _mm_fmadd_ps(a.c3.v, _mm_broadcast_ss(&b.c1.w), c1);

    //c2 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c2.x));
    //c2 = _mm_fmadd_ps(a.c1.v, _mm_broadcast_ss(&b.c2.y), c2);
    //c2 = _mm_fmadd_ps(a.c2.v, _mm_broadcast_ss(&b.c2.z), c2);
    //c2 = _mm_fmadd_ps(a.c3.v, _mm_broadcast_ss(&b.c2.w), c2);

    //c3 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c3.x));
    //c3 = _mm_fmadd_ps(a.c1.v, _mm_broadcast_ss(&b.c3.y), c3);
    //c3 = _mm_fmadd_ps(a.c2.v, _mm_broadcast_ss(&b.c3.z), c3);
    //c3 = _mm_fmadd_ps(a.c3.v, _mm_broadcast_ss(&b.c3.w), c3);

    _mm_store_ps((float *)&m.c0, c0);
    _mm_store_ps((float *)&m.c1, c1);
    _mm_store_ps((float *)&m.c2, c2);
    _mm_store_ps((float *)&m.c3, c3);

    return m;

#else
    return (HglmMat4) {
        /* c0 */
        .m00 = a.c0.x * b.c0.x + a.c1.x * b.c0.y + a.c2.x * b.c0.z + a.c3.x * b.c0.w,
        .m10 = a.c0.y * b.c0.x + a.c1.y * b.c0.y + a.c2.y * b.c0.z + a.c3.y * b.c0.w,
        .m20 = a.c0.z * b.c0.x + a.c1.z * b.c0.y + a.c2.z * b.c0.z + a.c3.z * b.c0.w,
        .m30 = a.c0.w * b.c0.x + a.c1.w * b.c0.y + a.c2.w * b.c0.z + a.c3.w * b.c0.w,
        /* c1 */
        .m01 = a.c0.x * b.c1.x + a.c1.x * b.c1.y + a.c2.x * b.c1.z + a.c3.x * b.c1.w,
        .m11 = a.c0.y * b.c1.x + a.c1.y * b.c1.y + a.c2.y * b.c1.z + a.c3.y * b.c1.w,
        .m21 = a.c0.z * b.c1.x + a.c1.z * b.c1.y + a.c2.z * b.c1.z + a.c3.z * b.c1.w,
        .m31 = a.c0.w * b.c1.x + a.c1.w * b.c1.y + a.c2.w * b.c1.z + a.c3.w * b.c1.w,
        /* c2 */
        .m02 = a.c0.x * b.c2.x + a.c1.x * b.c2.y + a.c2.x * b.c2.z + a.c3.x * b.c2.w,
        .m12 = a.c0.y * b.c2.x + a.c1.y * b.c2.y + a.c2.y * b.c2.z + a.c3.y * b.c2.w,
        .m22 = a.c0.z * b.c2.x + a.c1.z * b.c2.y + a.c2.z * b.c2.z + a.c3.z * b.c2.w,
        .m32 = a.c0.w * b.c2.x + a.c1.w * b.c2.y + a.c2.w * b.c2.z + a.c3.w * b.c2.w,
        /* c3 */
        .m03 = a.c0.x * b.c3.x + a.c1.x * b.c3.y + a.c2.x * b.c3.z + a.c3.x * b.c3.w,
        .m13 = a.c0.y * b.c3.x + a.c1.y * b.c3.y + a.c2.y * b.c3.z + a.c3.y * b.c3.w,
        .m23 = a.c0.z * b.c3.x + a.c1.z * b.c3.y + a.c2.z * b.c3.z + a.c3.z * b.c3.w,
        .m33 = a.c0.w * b.c3.x + a.c1.w * b.c3.y + a.c2.w * b.c3.z + a.c3.w * b.c3.w,
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_scale(HglmMat4 m, HglmVec3 v)
{
    return hglm_mat4_mul_mat4(m, hglm_mat4_make_scale(v));
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_rotate(HglmMat4 m, float angle, HglmVec3 axis)
{
    return hglm_mat4_mul_mat4(m, hglm_mat4_make_rotation(angle, axis));
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_translate(HglmMat4 m, HglmVec3 v)
{
    return hglm_mat4_mul_mat4(m, hglm_mat4_make_translation(v));
}

__attribute__ ((const, unused))
static HGL_INLINE HglmVec4 hglm_mat4_perspective_project(HglmMat4 proj, HglmVec4 v)
{
    HglmVec4 u = hglm_mat4_mul_vec4(proj, v);
    u.x /= u.w;
    u.y /= u.w;
    u.z /= u.w;
    return u;
}


/* ========== Arbitrary size Matrix funtions =================================*/

#define hglm_mat_at(m, y, x) ((m).data[(y)*(m).N + (x)])
#define hglm_mat_print(m) \
    do { \
        printf("%s = \n", #m ); \
        for (uint32_t row = 0; row < (m).M; row++) {\
            printf("  | "); \
            for (uint32_t col = 0; col < (m).N; col++) {\
                printf("%16f ", (double) hglm_mat_at((m), row, col)); \
            } \
            printf(" |\n"); \
        } \
    } while(0)

static HGL_INLINE HglmMat hglm_mat_make(uint32_t M /* rows */, uint32_t N /* cols */)
{
    HglmMat m = {
        .data = HGLM_ALLOC(M * N * sizeof(*m.data)),
        .M = M,
        .N = N,
    };
    assert(m.data != NULL);
    return m;
}

static HGL_INLINE HglmMat hglm_mat_make_identity(uint32_t N)
{
    HglmMat m = hglm_mat_make(N, N);
    hglm_mat_fill(m, 0);
    for (uint32_t i = 0; i < N; i++) {
        hglm_mat_at(m, i, i) = 1.0f;
    }
    return m;
}

static HGL_INLINE HglmMat hglm_mat_make_copy(HglmMat m)
{
    HglmMat copy = hglm_mat_make(m.M, m.N);
    memcpy(copy.data, m.data, m.M * m.N * sizeof(*m.data));
    return copy;
}

static HGL_INLINE void hglm_mat_free(HglmMat m)
{
    HGLM_FREE(m.data);
}

static HGL_INLINE void hglm_mat_fill(HglmMat m, float value)
{
    for (uint32_t row = 0; row < m.M; row++) {
        for (uint32_t col = 0; col < m.N; col++) {
            hglm_mat_at(m, row, col) = value;
        }
    }
}

static HGL_INLINE void hglm_mat_add(HglmMat res, HglmMat a, HglmMat b)
{
    assert(a.M == b.M);
    assert(a.N == b.N);
    assert(res.N == a.N);
    assert(res.M == a.M);
    for (uint32_t row = 0; row < res.M; row++) {
        for (uint32_t col = 0; col < res.N; col++) {
            hglm_mat_at(res, row, col) = hglm_mat_at(a, row, col) + hglm_mat_at(b, row, col);
        }
    }
}

static HGL_INLINE void hglm_mat_sub(HglmMat res, HglmMat a, HglmMat b)
{
    assert(a.M == b.M);
    assert(a.N == b.N);
    assert(res.N == a.N);
    assert(res.M == a.M);
    for (uint32_t row = 0; row < res.M; row++) {
        for (uint32_t col = 0; col < res.N; col++) {
            hglm_mat_at(res, row, col) = hglm_mat_at(a, row, col) - hglm_mat_at(b, row, col);
        }
    }
}

static HGL_INLINE void hglm_mat_mul_scalar(HglmMat m, float s)
{
    for (uint32_t row = 0; row < m.M; row++) {
        for (uint32_t col = 0; col < m.N; col++) {
            hglm_mat_at(m, row, col) *= s;
        }
    }
}

static HGL_INLINE void hglm_mat_mul_mat(HglmMat res, HglmMat a, HglmMat b)
{
    /* AxB x BxC ==> AxC*/
    assert(a.N == b.M);
    assert(res.M == a.M);
    assert(res.N == b.N);
    assert(res.data != a.data);
    assert(res.data != b.data);
    for (uint32_t row = 0; row < res.M; row++) {
        for (uint32_t col = 0; col < res.N; col++) {
            hglm_mat_at(res, row, col) = 0.0f;
            for (uint32_t i = 0; i < res.N; i++) {
                hglm_mat_at(res, row, col) += hglm_mat_at(a, row, i) * hglm_mat_at(b, i, col);
            }
        }
    }
}

static HGL_INLINE void hglm_mat_transpose_in_place(HglmMat m)
{
    assert((m.M == m.N) && "In-place transpose only supports square matrices");
    for (uint32_t row = 0; row < m.M - 1; row++) {
        for (uint32_t col = row + 1; col < m.N; col++) {
            float temp = hglm_mat_at(m, row, col);
            hglm_mat_at(m, row, col) = hglm_mat_at(m, col, row);
            hglm_mat_at(m, col, row) = temp;
        }
    }
}

static HGL_INLINE void hglm_mat_transpose(HglmMat t, HglmMat m)
{
    assert(t.M == m.N);
    assert(t.N == m.M);
    for (uint32_t row = 0; row < t.M; row++) {
        for (uint32_t col = 0; col < t.N; col++) {
            hglm_mat_at(t, row, col) = hglm_mat_at(m, col, row);
        }
    }
}

static HGL_INLINE float hglm_mat_inverse(HglmMat inv, HglmMat m)
{
#if 0
    assert((m.M == m.N));
    assert((inv.M == m.M));
    assert((inv.N == m.N));
    float det = 1.0f;
    const uint32_t N = m.N;
    const float EPSILON = 0.00001f;
    HglmMat mat = hglm_mat_make_copy(m); // Todo don't make a temporary copy?

    /* construct inv as the identity matrix */
    hglm_mat_fill(inv, 0);
    for (uint32_t i = 0; i < N; i++) {
        hglm_mat_at(inv, i, i) = 1.0f;
    }
    
    /* Apply Gauss-Jordan elimination */
    for (uint32_t i = 0; i < N; i++) {
        float pivot = hglm_mat_at(mat, i, i);

        if (fabsf(pivot) < EPSILON) {
            det = 0.0f;
            goto out;
        }

        det *= pivot;

        for (uint32_t j = 0; j < N; j++) {
            hglm_mat_at(mat, i, j) /= pivot;
            hglm_mat_at(inv, i, j) /= pivot;
        }

        for (uint32_t k = 0; k < N; k++) {
            if (k == i) {
                continue;
            }
            float s = hglm_mat_at(mat, k, i);
            for (uint32_t j = 0; j < N; j++) {
                hglm_mat_at(mat, k, j) -= s * hglm_mat_at(mat, i, j);
                hglm_mat_at(inv, k, j) -= s * hglm_mat_at(inv, i, j);
            }
        }
    }

out:
    hglm_mat_free(mat);
    return det;

#else
    assert((m.M == m.N));
    assert((inv.M == m.M));
    assert((inv.N == m.N));
    float det = 1.0f;
    const uint32_t N = m.N;
    const float EPSILON = 0.00001f;
    HglmMat mat = hglm_mat_make_copy(m); // Todo don't make a temporary copy?

    /* construct inv as the identity matrix */
    hglm_mat_fill(inv, 0);
    for (uint32_t i = 0; i < N; i++) {
        hglm_mat_at(inv, i, i) = 1.0f;
    }
    
    /* Apply Gauss-Jordan elimination */
    for (uint32_t i = 0; i < N; i++) {
        uint32_t p = i;
        for (uint32_t j = i + 1; j < N; j++) {
            if (fabsf(hglm_mat_at(mat, j, i)) > fabsf(hglm_mat_at(mat, p, i))) {
                p = j;
            }
        }

        if (fabsf(hglm_mat_at(mat, p, i)) < EPSILON) {
            det = 0.0f;
            goto out;
        }

        if (p != i) {
            float temp;
            for (uint32_t j = 0; j < N; j++) {
                temp = hglm_mat_at(mat, i, j);
                hglm_mat_at(mat, i, j) = hglm_mat_at(mat, p, j);
                hglm_mat_at(mat, p, j) = temp;
            }
            for (uint32_t j = 0; j < N; j++) {
                temp = hglm_mat_at(inv, i, j);
                hglm_mat_at(inv, i, j) = hglm_mat_at(inv, p, j);
                hglm_mat_at(inv, p, j) = temp;
            }
            det *= -1.0f;
        }

        float pivot = hglm_mat_at(mat, i, i);
        det *= pivot;

        for (uint32_t j = 0; j < N; j++) {
            hglm_mat_at(mat, i, j) /= pivot;
            hglm_mat_at(inv, i, j) /= pivot;
        }

        for (uint32_t k = 0; k < N; k++) {
            if (k == i) {
                continue;
            }
            float s = hglm_mat_at(mat, k, i);
            for (uint32_t j = 0; j < N; j++) {
                hglm_mat_at(mat, k, j) -= s * hglm_mat_at(mat, i, j);
                hglm_mat_at(inv, k, j) -= s * hglm_mat_at(inv, i, j);
            }
        }
    }

out:
    hglm_mat_print(mat);
    hglm_mat_free(mat);
    return det;
#endif
}


/* ========== scalar & misc. math functions ==================================*/

static HGL_INLINE float hglm_pid(float error, float last_error, float *i, 
                                 float Kp, float Ki, float Kd, float dt)
{
    *i += error * dt;
    float d = (error - last_error) / dt;
    float p = error;

    return Kp*p + Ki*(*i) + Kd*d;
}

static HGL_INLINE float hglm_lerp(float a, float b, float t)
{
    return (1.0f - t) * a + t * b; // value
}

static HGL_INLINE float hglm_ilerp(float a, float b, float value)
{
    return (value - a) / (b - a); // t
}

static HGL_INLINE float hglm_clamp(float min, float max, float value)
{
    //return fminf(fmaxf(min, value), max);
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static HGL_INLINE float hglm_remap(float in_min,
                                   float in_max,
                                   float out_min,
                                   float out_max,
                                   float value)
{
    float t = hglm_ilerp(in_min, in_max, value);
    return hglm_lerp(out_min, out_max, t);
}

static HGL_INLINE float hglm_smoothstep(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

static HGL_INLINE float hglm_smootherstep(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static HGL_INLINE float hglm_sinstep(float t)
{
    return -0.5f * cosf(t * (float)HGLM_PI) + 0.5f;
}

static HGL_INLINE float hglm_lerpsmooth(float a, float b, float dt, float omega)
{
    return b + (a - b) * exp2f(-dt/omega);
}

static HGL_INLINE float hglm_smoothmin_quadratic(float a, float b, float k)
{
    k *= 4.0f;
    float h = fmaxf(k - fabsf(a - b), 0.0) / k;
    return fminf(a, b) - h*h*k*0.25f;
}

static HGL_INLINE float hglm_smoothmin_sigmoid(float a, float b, float k)
{
    k *= logf(2.0);
    float x = b - a;
    return a + x / (1.0f - exp2f(x / k));
}

static HGL_INLINE HglmVec4 hglm_bezier3f(float t)
{
    HglmMat4 bezier3 = ((HglmMat4) {.m00 =  1.0f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
                                    .m10 = -3.0f, .m11 =  3.0f, .m12 =  0.0f, .m13 =  0.0f,
                                    .m20 =  3.0f, .m21 = -6.0f, .m22 =  3.0f, .m23 =  0.0f,
                                    .m30 = -1.0f, .m31 =  3.0f, .m32 = -3.0f, .m33 =  1.0f});
    HglmVec4 ts = (HglmVec4) {.x = 1.0f, .y = t, .z = t*t, .w = t*t*t};
    return (HglmVec4) {
        .x = hglm_vec4_dot(ts, bezier3.c0),
        .y = hglm_vec4_dot(ts, bezier3.c1),
        .z = hglm_vec4_dot(ts, bezier3.c2),
        .w = hglm_vec4_dot(ts, bezier3.c3),
    };
}

static HGL_INLINE HglmVec4 hglm_hermite3f(float t)
{
    HglmMat4 hermite3 = ((HglmMat4) {.m00 =  1.0f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
                                     .m10 =  0.0f, .m11 =  1.0f, .m12 =  0.0f, .m13 =  0.0f,
                                     .m20 = -3.0f, .m21 = -2.0f, .m22 =  3.0f, .m23 = -1.0f,
                                     .m30 =  2.0f, .m31 =  1.0f, .m32 = -2.0f, .m33 =  1.0f});
    HglmVec4 ts = (HglmVec4) {.x = 1.0f, .y = t, .z = t*t, .w = t*t*t};
    return (HglmVec4) {
        .x = hglm_vec4_dot(ts, hermite3.c0),
        .y = hglm_vec4_dot(ts, hermite3.c1),
        .z = hglm_vec4_dot(ts, hermite3.c2),
        .w = hglm_vec4_dot(ts, hermite3.c3),
    };
}

static HGL_INLINE float hglm_grad(int hash, float x, float y, float z)
{
    int h = hash & 15;
    float u = (h < 8) ? x : y;
    float v = (h < 4) ? y : ((h == 12) || (h == 14)) ? x : z;
    return (((h & 1) == 0) ? u : -u) +
           (((h & 2) == 0) ? v : -v);
}

static HGL_INLINE float hglm_perlin3D(float x, float y, float z)
{
    static const int P[512] = {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
        36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120,
        234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
        88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
        134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133,
        230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161,
        1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116,
        188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124,
        123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16,
        58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163,
        70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110,
        79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193,
        238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
        49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45,
        127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141,
        128, 195, 78, 66, 215, 61, 156, 180
    };

    int X = (int)floorf(x) & 255;
    int Y = (int)floorf(y) & 255;
    int Z = (int)floorf(z) & 255;
    x -= floorf(x);
    y -= floorf(y);
    z -= floorf(z);
    float u = hglm_smootherstep(x);
    float v = hglm_smootherstep(y);
    float w = hglm_smootherstep(z);
    int A  = P[X] + Y;
    int AA = P[A] + Z;
    int AB = P[A+1] + Z;
    int B  = P[X+1] + Y;
    int BA = P[B] + Z;
    int BB = P[B+1] + Z;
    return 0.5f + hglm_lerp(hglm_lerp(hglm_lerp(hglm_grad(P[AA  ], x    , y    , z    ),
                                                hglm_grad(P[BA  ], x - 1, y    , z    ), u),
                                      hglm_lerp(hglm_grad(P[AB  ], x    , y - 1, z    ),
                                                hglm_grad(P[BB  ], x - 1, y - 1, z    ), u), v),
                            hglm_lerp(hglm_lerp(hglm_grad(P[AA+1], x    , y    , z - 1),
                                                hglm_grad(P[BA+1], x - 1, y    , z - 1), u),
                                      hglm_lerp(hglm_grad(P[AB+1], x    , y - 1, z - 1),
                                                hglm_grad(P[BB+1], x - 1, y - 1, z - 1), u), v), w);
}


/**
 * Fast fourier transform.
 *
 * EXAMPLE:
 *
 *     #define HGL_FFT_IMPLEMENTATION
 *     #define HGL_FFT_USE_SIMD
 *     #include "hgl_fft.h"
 *
 *     #define N (1 << 20)
 *
 *     float signal[N];
 *     float complex signal_frequencies[N];
 *     float complex reconstructed_signal[N];
 *
 *     int main(void)
 *     {
 *         // Generate some signal
 *         for (int i = 0; i < N; i++) {
 *             float t = 1.0f * (float)i/N;
 *             signal[i] = sinf(1*2*PI*t) + sinf(2*2*PI*t) + cosf(3*2*PI*t);
 *         }
 *
 *         // Normalize (not necessary)
 *         float max = 0;
 *         for (int i = 0; i < N; i++) {
 *             max = (fabsf(signal[i]) > max) ? fabsf(signal[i]) : max;
 *         }
 *         for (int i = 0; i < N; i++) {
 *             signal[i] /= max;
 *         }
 *
 *         // Perform Fourier Transform on `signal`
 *         fft(signal, signal_frequencies, N);
 *
 *         // Do something with frequency domain data
 *         float cutoff_freq_hz = 120;
 *         high_pass_filter(signal_frequencies, cutoff_freq_hz, N);
 *
 *         // Reconstruct signal from frequencies.
 *         ifft(signal_frequencies, reconstructed_signal, N);
 *     }
 *
 */
static HGL_INLINE void hglm_fft(float in[], float complex out[], int n)
{
    assert((n & (n - 1)) == 0); // n is power of 2
    hglm_fft_internal_(in, out, n, 1);
}

static HGL_INLINE void hglm_ifft(float complex in[], float complex out[], int n)
{
    assert((n & (n - 1)) == 0); // n is power of 2
    hglm_ifft_internal_(in, out, n, 1);
    for (int i = 0; i < n; i++) {
        out[i] = out[i]/n;
    }
}

static void hglm_fft_internal_(float in[], float complex out[], int n, int stride)
{
    if (n == 1) {
        out[0] = in[0];
        return;
    }

    hglm_fft_internal_(in, out, n/2, 2*stride); // even
    hglm_fft_internal_(in + stride, out + n/2, n/2, 2*stride); // odd

#ifndef HGLM_USE_SIMD
    for(int k = 0; k < n/2; k++) {
        float w = -2*(float)HGLM_PI*((float)k/n);
        float complex v = out[k + n/2] * (cosf(w) + I*sinf(w));
        out[k + n/2]    = out[k] - v;
        out[k]          = out[k] + v;
    }
#else
    if (n/2 < 4) {
        for(int k = 0; k < n/2; k++) {
            float w = -2*(float)HGLM_PI*((float)k/n);
            float complex v = out[k + n/2] * (cosf(w) + I*sinf(w));
            //float complex v = out[k + n/2] * cexpf(-2*HGLM_PI*((float)k/n)*I);
            out[k + n/2]    = out[k] - v;
            out[k]          = out[k] + v;
        }
    } else {
        for(int k = 0; k < n/2; k += 4) {
            float ws[3]  __attribute__((aligned(16)));
            __m128 vec_ks      = _mm_set_ps1((float)(k));          // (k, k, k, k)
            __m128 vec_rns     = _mm_set_ps1(1.0f/n);              // (1/n, 1/n, 1/n, 1/n)
            __m128 vec_2pi     = _mm_set_ps1(-2*HGLM_PI);          // (2*HGLM_PI, 2*HGLM_PI, 2*HGLM_PI, 2*HGLM_PI)
            __m128 vec_offsets = _mm_set_ps(3, 2, 1, 0);           // (0, 1, 2, 3)
            __m128 vec_indices = _mm_add_ps(vec_ks, vec_offsets);  // (k, k+1, k+2, k+3)
            __m128 vec_ts      = _mm_mul_ps(vec_indices, vec_rns); // (k/n, (k+1)/n, (k+2)/n, (k+3)/n)
            __m128 vec_ws      = _mm_mul_ps(vec_ts, vec_2pi);      // (2*HGLM_PI*(k/n), 2*HGLM_PI*((k+1)/n), 2*HGLM_PI*((k+2)/n), 2*HGLM_PI*((k+3)/n))
            _mm_store_ps(ws, vec_ws);
            float complex v0 = out[k + n/2] * (cosf(ws[0]) + I*sinf(ws[0]));
            float complex v1 = out[k + n/2 + 1] * (cosf(ws[1]) + I*sinf(ws[1]));
            float complex v2 = out[k + n/2 + 2] * (cosf(ws[2]) + I*sinf(ws[2]));
            float complex v3 = out[k + n/2 + 3] * (cosf(ws[3]) + I*sinf(ws[3]));
            out[k + n/2]     = out[k] - v0;
            out[k]           = out[k] + v0;
            out[k + n/2 + 1] = out[k + 1] - v1;
            out[k + 1]       = out[k + 1] + v1;
            out[k + n/2 + 2] = out[k + 2] - v2;
            out[k + 2]       = out[k + 2] + v2;
            out[k + n/2 + 3] = out[k + 3] - v3;
            out[k + 3]       = out[k + 3] + v3;
        }
    }
#endif
}

static void hglm_ifft_internal_(float complex in[], float complex out[], int n, int stride)
{
    if (n == 1) {
        out[0] = in[0];
        return;
    }

    hglm_ifft_internal_(in, out, n/2, 2*stride); // even
    hglm_ifft_internal_(in + stride, out + n/2, n/2, 2*stride); // odd

#ifndef HGLM_USE_SIMD
    for(int k = 0; k < n/2; k++) {
        float w = 2*(float)HGLM_PI*((float)k/n);
        float complex v = out[k + n/2] * (cosf(w) + I*sinf(w));
        out[k + n/2]    = out[k] - v;
        out[k]          = out[k] + v;
    }
#else
    if (n/2 < 4) {
        for(int k = 0; k < n/2; k++) {
            float w = 2*(float)HGLM_PI*((float)k/n);
            float complex v = out[k + n/2] * (cosf(w) + I*sinf(w));
            out[k + n/2]    = out[k] - v;
            out[k]          = out[k] + v;
        }
    } else {
        for(int k = 0; k < n/2; k += 4) {
            float ws[3]  __attribute__((aligned(16)));
            __m128 vec_ks      = _mm_set_ps1((float)(k));          // (k, k, k, k)
            __m128 vec_rns     = _mm_set_ps1(1.0f/n);              // (1/n, 1/n, 1/n, 1/n)
            __m128 vec_2pi     = _mm_set_ps1(2*HGLM_PI);           // (2*HGLM_PI, 2*HGLM_PI, 2*HGLM_PI, 2*HGLM_PI)
            __m128 vec_offsets = _mm_set_ps(3, 2, 1, 0);           // (0, 1, 2, 3)
            __m128 vec_indices = _mm_add_ps(vec_ks, vec_offsets);  // (k, k+1, k+2, k+3)
            __m128 vec_ts      = _mm_mul_ps(vec_indices, vec_rns); // (k/n, (k+1)/n, (k+2)/n, (k+3)/n)
            __m128 vec_ws      = _mm_mul_ps(vec_ts, vec_2pi);      // (2*HGLM_PI*(k/n), 2*HGLM_PI*((k+1)/n), 2*HGLM_PI*((k+2)/n), 2*HGLM_PI*((k+3)/n))
            _mm_store_ps(ws, vec_ws);
            float complex v0 = out[k + n/2] * (cosf(ws[0]) + I*sinf(ws[0]));
            float complex v1 = out[k + n/2 + 1] * (cosf(ws[1]) + I*sinf(ws[1]));
            float complex v2 = out[k + n/2 + 2] * (cosf(ws[2]) + I*sinf(ws[2]));
            float complex v3 = out[k + n/2 + 3] * (cosf(ws[3]) + I*sinf(ws[3]));
            out[k + n/2]        = out[k] - v0;
            out[k]              = out[k] + v0;
            out[k + n/2 + 1]    = out[k + 1] - v1;
            out[k + 1]          = out[k + 1] + v1;
            out[k + n/2 + 2]    = out[k + 2] - v2;
            out[k + 2]          = out[k + 2] + v2;
            out[k + n/2 + 3]    = out[k + 3] - v3;
            out[k + 3]          = out[k + 3] + v3;
        }
    }
#endif

}

static HGL_INLINE float hglm_nm_newton_raphson(float (*f)(float), float (*df)(float), float x0)
{
    const float EPSILON = 0.00001f;
    const int MAX_ITERATIONS = 32;
    float x1;

    for (int n = 0; n < MAX_ITERATIONS; n++) {
        x1 = x0 - f(x0) / df(x0);
        if (fabsf(x1 - x0) < EPSILON) {
            break;
        }
        x0 = x1;
    }

    return x1;
}

static HGL_INLINE float hglm_nm_secant(float (*f)(float), float x0, float x1)
{
    const float EPSILON = 0.00001f;
    const int MAX_ITERATIONS = 32;
    float x2;

    for (int n = 0; n < MAX_ITERATIONS; n++) {
        x2 = x1 - (x1 - x0) / (f(x1) - f(x0)) * f(x1);
        if (fabsf(x1 - x0) < EPSILON) {
            break;
        }
        x0 = x1;
        x1 = x2;
    }

    return x2;
}

static HGL_INLINE float hglm_nm_trapezoidal(float (*f)(float), float a, float b, int N)
{
    float area = (f(a) + f(b)) / 2.0f;
    float h = (b - a) / (float)N;
    for (int i = 1; i < N; i++) {
        area += f(a + i * h);
    }
    area *= h;
    return area;
}

static HGL_INLINE float hglm_nm_simpson1(float (*f)(float), float a, float b, int N)
{
    assert((N & (N - 1)) == 0 && "Simpson's 1/3 rule requires an even number of sub-intervals.");
    float area = f(a) + f(b);
    float h = (b - a) / (float)N;
    for (int i = 1; i < N; i++) {
        if ((i & 1) == 0) {
            area += 2.0f * f(a + i * h);
        } else {
            area += 4.0f * f(a + i * h);
        }
    }
    area *= (h / 3);
    return area;
}

static HGL_INLINE float hglm_nm_euler(float (*f)(float, float), float x0, float y0, float xn, float h)
{
#ifdef HGLM_NM_USE_KAHAN_COMPENSATION
    float x = x0;
    float y = y0;
    float dy, y_next;
    float c = 0.0f; // Kahan compensation
    const int N = (int)fabsf((xn - x0) / h);

    for (int i = 0; i < N; i++) {
        dy = h * f(x, y) - c;
        y_next = y + dy;
        c = (y_next - y) - dy;
        y = y_next; 
        x = x0 + h*i;
    }

    return y;
#else
    float x = x0;
    float y = y0;
    const int N = (int)fabsf((xn - x0) / h);

    for (int i = 0; i < N; i++) {
        y = y + h * f(x, y); 
        x = x0 + h*i;
    }

    return y;
#endif
}

static HGL_INLINE float hglm_nm_runge_kutta4(float (*f)(float, float), float x0, float y0, float xn, float h)
{
#ifdef HGLM_NM_USE_KAHAN_COMPENSATION
    float x = x0;
    float y = y0;
    float k1, k2, k3, k4;
    float dy, y_next;
    float c = 0.0f; // Kahan compensation
    const int N = (int)fabsf((xn - x0) / h);

    for (int i = 0; i < N; i++) {
        k1 = f(x, y);
        k2 = f(x + (h/2.0f), y + k1 * (h/2.0f));
        k3 = f(x + (h/2.0f), y + k2 * (h/2.0f));
        k4 = f(x + h, y + k3 * h);
        dy = (h/6) * (k1 + 2.0f * k2 + 2.0f * k3 + k4) - c;
        y_next = y + dy;
        c = (y_next - y) - dy;
        y = y_next;
        x = x0 + h*i;
    }

    return y;
#else
    float x = x0;
    float y = y0;
    float k1, k2, k3, k4;
    const int N = (int)fabsf((xn - x0) / h);

    for (int i = 0; i < N; i++) {
        k1 = f(x, y);
        k2 = f(x + (h/2.0f), y + k1 * (h/2.0f));
        k3 = f(x + (h/2.0f), y + k2 * (h/2.0f));
        k4 = f(x + h, y + k3 * h);
        y = y + (h/6) * (k1 + 2.0f * k2 + 2.0f * k3 + k4); 
        x = x0 + h*i;
    }

    return y;
#endif
}

static HGL_INLINE float hglm_nm_gauss_jordan_solver(HglmMat A, HglmMat x, HglmMat b)
{
    assert(A.N == x.M);
    assert(x.M == b.M);
    assert(x.N == 1);
    assert(b.N == 1);

    float det = 1.0f;
    const float EPSILON = 0.00001f;

    HglmMat m = hglm_mat_make_copy(A); // Todo don't make a temporary copy?

    /* Construct x as a copy of b */
    for (uint32_t i = 0; i < b.M; i++) {
        hglm_mat_at(x, i, 0) = hglm_mat_at(b, i, 0);
    }

    /* Apply Gauss-Jordan elimination */
    for (uint32_t i = 0; i < m.M; i++) {
        uint32_t p = i;
        for (uint32_t j = i + 1; j < m.M; j++) {
            if (fabsf(hglm_mat_at(m, j, i)) > fabsf(hglm_mat_at(m, p, i))) {
                p = j;
            }
        }

        if (fabsf(hglm_mat_at(m, p, i)) < EPSILON) {
            det = 0.0f;
            goto out;
        }

        if (p != i) {
            float temp;
            for (uint32_t j = 0; j < m.N; j++) {
                temp = hglm_mat_at(m, i, j);
                hglm_mat_at(m, i, j) = hglm_mat_at(m, p, j);
                hglm_mat_at(m, p, j) = temp;
            }
            temp = hglm_mat_at(x, i, 0);
            hglm_mat_at(x, i, 0) = hglm_mat_at(x, p, 0);
            hglm_mat_at(x, p, 0) = temp;
            det *= -1.0f;
        }

        float pivot = hglm_mat_at(m, i, i);
        det *= pivot;

        for (uint32_t j = 0; j < m.N; j++) {
            hglm_mat_at(m, i, j) /= pivot;
        }
        hglm_mat_at(x, i, 0) /= pivot;

        for (uint32_t k = 0; k < m.M; k++) {
            if (k == i) {
                continue;
            }
            float s = hglm_mat_at(m, k, i);
            for (uint32_t j = 0; j < m.N; j++) {
                hglm_mat_at(m, k, j) -= s * hglm_mat_at(m, i, j);
            }
            hglm_mat_at(x, k, 0) -= s * hglm_mat_at(x, i, 0);
        }
    }

out:
    hglm_mat_free(m);
    return det;
}

#endif /* HGLM_H */

#ifdef HGLM_STRIP_PREFIX
 
#ifndef PI
#define PI HGLM_PI
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD HGLM_DEG_TO_RAD
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG HGLM_RAD_TO_DEG
#endif

#ifndef MAT2_IDENTITY
#define MAT2_IDENTITY HGLM_MAT2_IDENTITY
#endif

#ifndef MAT3_IDENTITY
#define MAT3_IDENTITY HGLM_MAT3_IDENTITY
#endif

#ifndef MAT4_IDENTITY
#define MAT4_IDENTITY HGLM_MAT4_IDENTITY
#endif

typedef HglmIVec2 IVec2;
typedef HglmIVec3 IVec3;
typedef HglmIVec4 IVec4;
typedef HglmVec2   Vec2;
typedef HglmVec3   Vec3;
typedef HglmVec4   Vec4;
typedef HglmQuat   Quat;
typedef HglmMat2   Mat2;
typedef HglmMat3   Mat3;
typedef HglmMat4   Mat4;
typedef HglmMat    Mat;

#define ivec2_print              hglm_ivec2_print
#define ivec2                    hglm_ivec2
#define ivec2_add                hglm_ivec2_add
#define ivec2_sub                hglm_ivec2_sub
#define ivec2_distance           hglm_ivec2_distance
#define ivec2_length             hglm_ivec2_length
#define ivec2_mul_scalar         hglm_ivec2_mul_scalar
#define ivec2_lerp               hglm_ivec2_lerp

#define ivec3_print              hglm_ivec3_print
#define ivec3                    hglm_ivec3
#define ivec3_add                hglm_ivec3_add

#define ivec4_print              hglm_ivec4_print
#define ivec4                    hglm_ivec4

#define vec2_print               hglm_vec2_print
#define vec2                     hglm_vec2
#define vec2_from_polar          hglm_vec2_from_polar
#define vec2_add                 hglm_vec2_add
#define vec2_sub                 hglm_vec2_sub
#define vec2_distance            hglm_vec2_distance
#define vec2_length              hglm_vec2_length
#define vec2_normalize           hglm_vec2_normalize
#define vec2_dot                 hglm_vec2_dot
#define vec2_wedge               hglm_vec2_wedge
#define vec2_recip               hglm_vec2_recip
#define vec2_hadamard            hglm_vec2_hadamard
#define vec2_mul_scalar          hglm_vec2_mul_scalar
#define vec2_reflect             hglm_vec2_reflect
#define vec2_lerp                hglm_vec2_lerp
#define vec2_slerp               hglm_vec2_slerp
#define vec2_bezier3             hglm_vec2_bezier3

#define vec3_print               hglm_vec3_print
#define vec3                     hglm_vec3
#define vec3_from_spherical      hglm_vec3_from_spherical
#define vec3_add                 hglm_vec3_add
#define vec3_sub                 hglm_vec3_sub
#define vec3_distance            hglm_vec3_distance
#define vec3_length              hglm_vec3_length
#define vec3_normalize           hglm_vec3_normalize
#define vec3_dot                 hglm_vec3_dot
#define vec3_cross               hglm_vec3_cross
#define vec3_recip               hglm_vec3_recip
#define vec3_hadamard            hglm_vec3_hadamard
#define vec3_mul_scalar          hglm_vec3_mul_scalar
#define vec3_reflect             hglm_vec3_reflect
#define vec3_lerp                hglm_vec3_lerp
#define vec3_slerp               hglm_vec3_slerp
#define vec3_bezier3             hglm_vec3_bezier3

#define vec4_print               hglm_vec4_print
#define vec4                     hglm_vec4
#define vec4_add                 hglm_vec4_add
#define vec4_sub                 hglm_vec4_sub
#define vec4_distance            hglm_vec4_distance
#define vec4_length              hglm_vec4_length
#define vec4_normalize           hglm_vec4_normalize
#define vec4_dot                 hglm_vec4_dot
#define vec4_recip               hglm_vec4_recip
#define vec4_hadamard            hglm_vec4_hadamard
#define vec4_mul_scalar          hglm_vec4_mul_scalar
#define vec4_swizzle             hglm_vec4_swizzle
#define vec4_perspective_divide  hglm_vec4_perspective_divide
#define vec4_lerp                hglm_vec4_lerp
#define vec4_bezier3             hglm_vec4_bezier3

#define quat                     hglm_quat
#define quat_from_angle_axis     hglm_quat_from_angle_axis
#define quat_mul_quat            hglm_quat_mul_quat
#define quat_mul_scalar          hglm_quat_mul_scalar
#define quat_pow                 hglm_quat_pow
#define quat_length              hglm_quat_length
#define quat_square_length       hglm_quat_square_length
#define quat_conjugate           hglm_quat_conjugate
#define quat_inverse             hglm_quat_inverse
#define quat_apply_vec3          hglm_quat_apply_vec3
#define quat_slerp               hglm_quat_slerp

#define mat2_print               hglm_mat2_print
#define mat2                     hglm_mat2
#define mat2_identity            hglm_mat2_identity
#define mat2_make_scale          hglm_mat2_make_scale
#define mat2_make_rotation       hglm_mat2_make_rotation
#define mat2_adjugate            hglm_mat2_adjugate
#define mat2_inverse             hglm_mat2_inverse
#define mat2_add                 hglm_mat2_add
#define mat2_sub                 hglm_mat2_sub
#define mat2_mul_scalar          hglm_mat2_mul_scalar
#define mat2_mul_vec2            hglm_mat2_mul_vec2
#define mat2_mul_mat2            hglm_mat2_mul_mat2
#define mat2_scale               hglm_mat2_scale
#define mat2_rotate              hglm_mat2_rotate
#define mat2_determinant         hglm_mat2_determinant

#define mat3_print               hglm_mat3_print
#define mat3                     hglm_mat3
#define mat3_identity            hglm_mat3_identity
#define mat3_make_scale          hglm_mat3_make_scale
#define mat3_make_rotation       hglm_mat3_make_rotation
#define mat3_make_translation    hglm_mat3_make_translation
#define mat3_demote_from_mat4    hglm_mat3_demote_from_mat4
#define mat3_adjugate            hglm_mat3_adjugate
#define mat3_inverse             hglm_mat3_inverse
#define mat3_affine_inverse      hglm_mat3_affine_inverse
#define mat3_transpose           hglm_mat3_transpose
#define mat3_add                 hglm_mat3_add
#define mat3_sub                 hglm_mat3_sub
#define mat3_mul_scalar          hglm_mat3_mul_scalar
#define mat3_mul_vec3            hglm_mat3_mul_vec3
#define mat3_mul_mat3            hglm_mat3_mul_mat3
#define mat3_scale               hglm_mat3_scale
#define mat3_rotate              hglm_mat3_rotate
#define mat3_translate           hglm_mat3_translate
#define mat3_determinant         hglm_mat3_determinant

#define mat4_print               hglm_mat4_print
#define mat4                     hglm_mat4
#define mat4_zero                hglm_mat4_zero
#define mat4_identity            hglm_mat4_identity
#define mat4_make_scale          hglm_mat4_make_scale
#define mat4_make_rotation       hglm_mat4_make_rotation
#define mat4_make_translation    hglm_mat4_make_translation
#define mat4_promote_from_mat3   hglm_mat4_promote_from_mat3
#define mat4_ortho               hglm_mat4_ortho
#define mat4_ortho_inverse       hglm_mat4_ortho_inverse
#define mat4_perspective         hglm_mat4_perspective
#define mat4_affine_inverse      hglm_mat4_affine_inverse
#define mat4_transpose           hglm_mat4_transpose
#define mat4_look_at             hglm_mat4_look_at
#define mat4_look_to             hglm_mat4_look_to
#define mat4_add                 hglm_mat4_add
#define mat4_sub                 hglm_mat4_sub
#define mat4_mul_scalar          hglm_mat4_mul_scalar
#define mat4_mul_vec4            hglm_mat4_mul_vec4
#define mat4_mul_mat4            hglm_mat4_mul_mat4
#define mat4_scale               hglm_mat4_scale
#define mat4_rotate              hglm_mat4_rotate
#define mat4_translate           hglm_mat4_translate
#define mat4_perspective_project hglm_mat4_perspective_project

#define mat_print                hglm_mat_print
#define mat_at                   hglm_mat_at
#define mat_make                 hglm_mat_make
#define mat_make_identity        hglm_mat_make_identity
#define mat_make_copy            hglm_mat_make_copy
#define mat_free                 hglm_mat_free
#define mat_fill                 hglm_mat_fill
#define mat_add                  hglm_mat_add
#define mat_sub                  hglm_mat_sub
#define mat_mul_scalar           hglm_mat_mul_scalar
#define mat_mul_mat              hglm_mat_mul_mat
#define mat_transpose_in_place   hglm_mat_transpose_in_place
#define mat_transpose            hglm_mat_transpose
#define mat_inverse              hglm_mat_inverse

#define pid                      hglm_pid
#define lerp                     hglm_lerp
#define ilerp                    hglm_ilerp
#define clamp                    hglm_clamp
#define remap                    hglm_remap
#define smoothstep               hglm_smoothstep
#define smootherstep             hglm_smootherstep
#define sinstep                  hglm_sinstep
#define lerpsmooth               hglm_lerpsmooth
#define smoothmin_quadratic      hglm_smoothmin_quadratic
#define smoothmin_sigmoid        hglm_smoothmin_sigmoid
#define bezier3f                 hglm_bezier3f
#define hermite3f                hglm_hermite3f
#define perlin3D                 hglm_perlin3D

#define fft                      hglm_fft
#define ifft                     hglm_ifft

#define newton_raphson           hglm_nm_newton_raphson
#define secant                   hglm_nm_secant
#define trapezoidal              hglm_nm_trapezoidal
#define simpson1                 hglm_nm_simpson1
#define euler                    hglm_nm_euler
#define runge_kutta4             hglm_nm_runge_kutta4
#define gauss_jordan_solver      hglm_nm_gauss_jordan_solver

#endif /* HGLM_STRIP_PREFIX */


// TODO IVec2, IVec3, IVec4 functions
// TODO Mat2 functions

