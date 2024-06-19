#ifndef HGL_MATH_H
#define HGL_MATH_H

#include <math.h>

#include <assert.h> // DEBUG
#include <stdbool.h> // DEBUG

#define HGL_INLINE inline __attribute__((always_inline))

#ifndef PI
#define PI 3.14159265358979
#endif

#ifdef HGL_MATH_USE_SIMD
#   include <smmintrin.h>
#   include <immintrin.h>
#endif

#define MAT4_IDENTITY ((mat4) { .m00 = 1.0f, .m01 = 0.0f, .m02 = 0.0f, .m03 = 0.0f, \
                                .m10 = 0.0f, .m11 = 1.0f, .m12 = 0.0f, .m13 = 0.0f, \
                                .m20 = 0.0f, .m21 = 0.0f, .m22 = 1.0f, .m23 = 0.0f, \
                                .m30 = 0.0f, .m31 = 0.0f, .m32 = 0.0f, .m33 = 1.0f,})

typedef struct
{
    float x;
    float y;
} vec2;

typedef struct
{
    union {
        struct {
            float x;
            float y;
        };
        vec2 xy;
    };
    float z;
} vec3;

//typedef struct
//{
//    union {
//        struct {
//            union {
//                struct {
//                    union {
//                        struct {
//                            float x;
//                            float y;
//                        };
//                        vec2 xy;
//                    };
//                    float z;
//                };
//                vec3 xyz;
//            };
//            float w;
//        };
//        __m128 v;
//    };
//} vec4;


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
                    vec2 xy;
                };
                float z;
            };
            vec3 xyz;
        };
        float w;
    };
#ifdef HGL_MATH_USE_SIMD
    __m128 v;
#endif
} vec4;

typedef struct __attribute__ ((aligned(16)))
{
    union {
        struct {
            vec4 c0;
            vec4 c1;
            vec4 c2;
            vec4 c3;
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
} mat4;

/* ========== vec2 ===========================================================*/

static HGL_INLINE vec2 vec2_make(float x, float y)
{
    return (vec2){.x = x, .y = y};
}

static HGL_INLINE vec2 vec2_add(vec2 a, vec2 b)
{
    return (vec2){.x = a.x + b.x, .y = a.y + b.y};
}

static HGL_INLINE vec2 vec2_sub(vec2 a, vec2 b)
{
    return (vec2){.x = a.x - b.x, .y = a.y - b.y};
}

static HGL_INLINE float vec2_distance(vec2 a, vec2 b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx*dx + dy*dy);
}

static HGL_INLINE float vec2_len(vec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static HGL_INLINE vec2 vec2_normalize(vec2 v)
{
    float ilen = 1.0f / vec2_len(v);
    return (vec2) {.x = v.x * ilen, .y = v.y * ilen};
}

static HGL_INLINE float vec2_dot(vec2 a, vec2 b)
{
    return a.x * b.x + a.y *b.y;
}

static HGL_INLINE vec2 vec2_hadamard(vec2 a, vec2 b)
{
    return (vec2) {.x = a.x * b.x, .y = a.y * b.y};
}

static HGL_INLINE vec2 vec2_mul_scalar(vec2 v, float s)
{
    return (vec2) {.x = s * v.x, .y = s * v.y};
}

static HGL_INLINE vec2 vec2_reflect(vec2 v, vec2 normal)
{
    return vec2_sub(v, vec2_mul_scalar(normal, 2*vec2_dot(v, normal)));
}

static HGL_INLINE vec2 vec2_lerp(vec2 a, vec2 b, float amount)
{
    return vec2_add(
        vec2_mul_scalar(a, 1.0f - amount),
        vec2_mul_scalar(b, amount)
    );
}

#define vec2_print(v) (printf("%s = {%f, %f}\n", #v , (v).x, (v).y))

/* ========== vec3 ===========================================================*/

static HGL_INLINE vec3 vec3_make(float x, float y, float z)
{
    return (vec3){.x = x, .y = y, .z = z};
}

static HGL_INLINE vec3 vec3_add(vec3 a, vec3 b)
{
    return (vec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

static HGL_INLINE vec3 vec3_sub(vec3 a, vec3 b)
{
    return (vec3){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

static HGL_INLINE float vec3_distance(vec3 a, vec3 b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

static HGL_INLINE float vec3_len(vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static HGL_INLINE vec3 vec3_normalize(vec3 v)
{
    float ilen = 1.0f / vec3_len(v);
    return (vec3) {.x = v.x * ilen, .y = v.y * ilen, .z = v.z * ilen};
}

static HGL_INLINE float vec3_dot(vec3 a, vec3 b)
{
    return a.x * b.x + a.y *b.y + a.z * b.z;
}

static HGL_INLINE vec3 vec3_cross(vec3 a, vec3 b)
{
    return (vec3) {
        .x = (a.y * b.z) - (a.z * b.y),
        .y = (a.z * b.x) - (a.x * b.z),
        .z = (a.x * b.y) - (a.y * b.x)
    };
}

static HGL_INLINE vec3 vec3_hadamard(vec3 a, vec3 b)
{
    return (vec3) {.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z};
}

static HGL_INLINE vec3 vec3_mul_scalar(vec3 v, float s)
{
    return (vec3) {.x = s * v.x, .y = s * v.y, .z = s * v.z};
}

static HGL_INLINE vec3 vec3_reflect(vec3 v, vec3 normal)
{
    return vec3_sub(v, vec3_mul_scalar(normal, 2*vec3_dot(v, normal)));
}

static HGL_INLINE vec3 vec3_lerp(vec3 a, vec3 b, float amount)
{
    return vec3_add(
        vec3_mul_scalar(a, 1.0f - amount),
        vec3_mul_scalar(b, amount)
    );
}

#define vec3_print(v) (printf("%s = {%f, %f, %f}\n", #v , (v).x, (v).y, (v).z))

/* ========== vec4 ===========================================================*/

static HGL_INLINE vec4 vec4_make(float x, float y, float z, float w)
{
#ifdef HGL_MATH_USE_SIMD
    return (vec4){.v = _mm_set_ps(w, z, y, x)};
#else
    return (vec4){.x = x, .y = y, .z = z, .w = w};
#endif
}

static HGL_INLINE vec4 vec4_add(vec4 a, vec4 b)
{
#ifdef HGL_MATH_USE_SIMD
    return (vec4){.v = _mm_add_ps(a.v, b.v)};
#else
    return (vec4){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w};
#endif
}

static HGL_INLINE vec4 vec4_sub(vec4 a, vec4 b)
{
#ifdef HGL_MATH_USE_SIMD
    return (vec4){.v = _mm_sub_ps(a.v, b.v)};
#else
    return (vec4){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z, .w = a.w - b.w};
#endif
}

static HGL_INLINE float vec4_distance(vec4 a, vec4 b)
{
#ifdef HGL_MATH_USE_SIMD
    printf("simd!\n");
    __m128 d = _mm_sub_ps(b.v, a.v);
    d = _mm_mul_ps(d, d);
    /* SSE3 */
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

static HGL_INLINE float vec4_len(vec4 v)
{
#ifdef HGL_MATH_USE_SIMD
    __m128 d = _mm_mul_ps(v.v, v.v);
    d = _mm_hadd_ps(d,d);
    d = _mm_hadd_ps(d,d);
    return sqrtf(_mm_cvtss_f32(d));
#else
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
#endif
}

static HGL_INLINE vec4 vec4_normalize(vec4 v)
{
#ifdef HGL_MATH_USE_SIMD
    __m128 rlen = _mm_set1_ps(1.0f / vec4_len(v));
    return (vec4) {.v = _mm_mul_ps(v.v, rlen)};
#else
    float len = vec4_len(v);
    return (vec4) {.x = v.x / len, .y = v.y / len, .z = v.z / len, .w = v.w / len};
#endif
}

static HGL_INLINE float vec4_dot(vec4 a, vec4 b)
{
#ifdef HGL_MATH_USE_SIMD
    __m128 v = _mm_mul_ps(a.v, b.v);
    v = _mm_hadd_ps(v,v);
    v = _mm_hadd_ps(v,v);
    return _mm_cvtss_f32(v);
#else
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
#endif
}

static HGL_INLINE vec4 vec4_hadamard(vec4 a, vec4 b)
{
#ifdef HGL_MATH_USE_SIMD
    return (vec4) {.v = _mm_mul_ps(a.v, b.v)};
#else
    return (vec4) {.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z, .w = a.w * b.w};
#endif
}

static HGL_INLINE vec4 vec4_mul_scalar(vec4 v, float s)
{
#ifdef HGL_MATH_USE_SIMD
    return (vec4) {.v = _mm_mul_ps(v.v, _mm_set1_ps(s))};
#else
    return (vec4) {.x = s * v.x, .y = s * v.y, .z = s * v.z, .w = s * v.w};
#endif
}

static HGL_INLINE vec4 vec4_lerp(vec4 a, vec4 b, float amount)
{
    return vec4_add(
        vec4_mul_scalar(a, 1.0f - amount),
        vec4_mul_scalar(b, amount)
    );
}

#define vec4_print(v) (printf("%s = {%f, %f, %f, %f}\n", #v , (v).x, (v).y, (v).z, (v).w))

/* ========== mat4 ===========================================================*/

static HGL_INLINE mat4 mat4_make(vec4 c0, vec4 c1, vec4 c2, vec4 c3)
{
    return (mat4){.c0 = c0, .c1 = c1, .c2 = c2, .c3 = c3};
}

static HGL_INLINE mat4 mat4_make_identity(void)
{
    return MAT4_IDENTITY;
}

//__attribute__ ((const, unused))
static HGL_INLINE mat4 mat4_make_scale(vec3 v)
{
    mat4 s = MAT4_IDENTITY;
    s.c0.x = v.x;
    s.c1.y = v.y;
    s.c2.z = v.z;
    return s;
}

//__attribute__ ((const, unused))
static inline mat4 mat4_make_rotation(float angle, vec3 axis)
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
    return (mat4) {
        .c0 = {.x =  c0x, .y =  c0y, .z =  c0z, .w = 0.0f},
        .c1 = {.x =  c1x, .y =  c1y, .z =  c1z, .w = 0.0f},
        .c2 = {.x =  c2x, .y =  c2y, .z =  c2z, .w = 0.0f},
        .c3 = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f},
    };
}

//__attribute__ ((pure, unused))
static HGL_INLINE mat4 mat4_make_translation(vec3 v)
{
    mat4 t = MAT4_IDENTITY;
    t.c3.x = v.x;
    t.c3.y = v.y;
    t.c3.z = v.z;
    return t;
}

//__attribute__ ((const, unused))
static HGL_INLINE mat4 mat4_make_ortho(float left, float right, float bottom,
                                       float top,  float near,  float far)
{
    mat4 m = MAT4_IDENTITY;
    m.c0.x =  2 / (right - left);
    m.c1.y =  2 / (top - bottom);
    m.c2.z = -2 / (far - near); // Note: inversion
    m.c3.x = -((right + left) / (right - left));
    m.c3.y = -((top + bottom) / (top - bottom));
    m.c3.z = -((far + near)   / (far - near));
    return m;
}

//__attribute__ ((const, unused))
static HGL_INLINE mat4 mat4_make_perspective(float fov, float aspect, float znear, float zfar)
{
    float a = 1.0f / aspect;
    float f = 1.0f / tanf(fov/2);
    float d0 = -(zfar + znear) / (zfar - znear);
    float d1 = -(2 * zfar * znear) / (zfar - znear);
    return (mat4) {
        .m00 =   a*f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
        .m10 =  0.0f, .m11 =     f, .m12 =  0.0f, .m13 =  0.0f,
        .m20 =  0.0f, .m21 =  0.0f, .m22 =    d0, .m23 =    d1,
        .m30 =  0.0f, .m31 =  0.0f, .m32 = -1.0f, .m33 =  0.0f,
    };
}

//__attribute__ ((pure, unused))
static HGL_INLINE mat4 mat4_mul_scalar(mat4 m, float s)
{
#ifdef HGL_MATH_USE_SIMD
    __m128 vec_s = _mm_set1_ps(s);
    return (mat4) {
        .c0 = {.v = _mm_mul_ps(vec_s, m.c0.v)},
        .c1 = {.v = _mm_mul_ps(vec_s, m.c1.v)},
        .c2 = {.v = _mm_mul_ps(vec_s, m.c2.v)},
        .c3 = {.v = _mm_mul_ps(vec_s, m.c3.v)}
    };
#else
    return (mat4) {
        .c0 = {.x = s * m.c0.x, .y = s * m.c0.y, .z = s * m.c0.z, .w = s * m.c0.w}, // c0
        .c1 = {.x = s * m.c1.x, .y = s * m.c1.y, .z = s * m.c1.z, .w = s * m.c1.w}, // c1
        .c2 = {.x = s * m.c2.x, .y = s * m.c2.y, .z = s * m.c2.z, .w = s * m.c2.w}, // c2
        .c3 = {.x = s * m.c3.x, .y = s * m.c3.y, .z = s * m.c3.z, .w = s * m.c3.w}  // c3
    };
#endif
}

//__attribute__ ((pure, unused))
static HGL_INLINE vec4 mat4_mul_vec4(mat4 m, vec4 v)
{
#ifdef HGL_MATH_USE_SIMD
    //(void) m;
    //(void) v;
    //return vec4_make(0,0,0,0);

    //__m128 vec_s = _mm_set_ps1(s);
    vec4 res;
    __m128 vec_vx = _mm_set1_ps(v.x);
    __m128 vec_vy = _mm_set1_ps(v.y);
    __m128 vec_vz = _mm_set1_ps(v.z);
    __m128 vec_vw = _mm_set1_ps(v.w);
    __m128 t0 = _mm_mul_ps(vec_vx, m.c0.v);
    __m128 t1 = _mm_mul_ps(vec_vy, m.c1.v);
    __m128 t2 = _mm_mul_ps(vec_vz, m.c2.v);
    __m128 t3 = _mm_mul_ps(vec_vw, m.c3.v);
    res.v = _mm_add_ps(_mm_add_ps(t0, t1),
                       _mm_add_ps(t2, t3));
    return res;
#else
    return (vec4) {
        .x = m.c0.x * v.x + m.c1.x * v.y + m.c2.x * v.z + m.c3.x * v.w,
        .y = m.c0.y * v.x + m.c1.y * v.y + m.c2.y * v.z + m.c3.y * v.w,
        .z = m.c0.z * v.x + m.c1.z * v.y + m.c2.z * v.z + m.c3.z * v.w,
        .w = m.c0.w * v.x + m.c1.w * v.y + m.c2.w * v.z + m.c3.w * v.w,
    };
#endif
}

//__attribute__ ((pure, unused))
static HGL_INLINE mat4 mat4_matmul4(mat4 a, mat4 b)
{
#ifdef HGL_MATH_USE_SIMD
    //mat4 res;
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
    __m128 bcx, bcy, bcz, bcw;
    __m128 t0, t1, t2, t3;
    __m128 c0, c1, c2, c3;

    /* c0 */
    bcx = _mm_set1_ps(b.c0.x);
    bcy = _mm_set1_ps(b.c0.y);
    bcz = _mm_set1_ps(b.c0.z);
    bcw = _mm_set1_ps(b.c0.w);
    t0 = _mm_mul_ps(a.c0.v, bcx);
    t1 = _mm_mul_ps(a.c1.v, bcy);
    t2 = _mm_mul_ps(a.c2.v, bcz);
    t3 = _mm_mul_ps(a.c3.v, bcw);
    c0 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c1 */
    bcx = _mm_set1_ps(b.c1.x);
    bcy = _mm_set1_ps(b.c1.y);
    bcz = _mm_set1_ps(b.c1.z);
    bcw = _mm_set1_ps(b.c1.w);
    t0 = _mm_mul_ps(a.c0.v, bcx);
    t1 = _mm_mul_ps(a.c1.v, bcy);
    t2 = _mm_mul_ps(a.c2.v, bcz);
    t3 = _mm_mul_ps(a.c3.v, bcw);
    c1 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c2 */
    bcx = _mm_set1_ps(b.c2.x);
    bcy = _mm_set1_ps(b.c2.y);
    bcz = _mm_set1_ps(b.c2.z);
    bcw = _mm_set1_ps(b.c2.w);
    t0 = _mm_mul_ps(a.c0.v, bcx);
    t1 = _mm_mul_ps(a.c1.v, bcy);
    t2 = _mm_mul_ps(a.c2.v, bcz);
    t3 = _mm_mul_ps(a.c3.v, bcw);
    c2 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c3 */
    bcx = _mm_set1_ps(b.c3.x);
    bcy = _mm_set1_ps(b.c3.y);
    bcz = _mm_set1_ps(b.c3.z);
    bcw = _mm_set1_ps(b.c3.w);
    t0 = _mm_mul_ps(a.c0.v, bcx);
    t1 = _mm_mul_ps(a.c1.v, bcy);
    t2 = _mm_mul_ps(a.c2.v, bcz);
    t3 = _mm_mul_ps(a.c3.v, bcw);
    c3 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    return (mat4) {
        .c0.v = c0,
        .c1.v = c1,
        .c2.v = c2,
        .c3.v = c3
    };
#else
    return (mat4) {
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

//__attribute__ ((const, unused))
static HGL_INLINE mat4 mat4_scale(mat4 m, vec3 v)
{
    return mat4_matmul4(m, mat4_make_scale(v));
}

//__attribute__ ((const, unused))
static inline mat4 mat4_rotate(mat4 m, float angle, vec3 axis)
{
    return mat4_matmul4(m, mat4_make_rotation(angle, axis));
}

//__attribute__ ((pure, unused))
static HGL_INLINE mat4 mat4_translate(mat4 m, vec3 v)
{
    return mat4_matmul4(m, mat4_make_translation(v));
}

//__attribute__ ((const, unused))
static HGL_INLINE vec4 mat4_perspective_project(mat4 proj, vec4 v)
{
    vec4 u = mat4_mul_vec4(proj, v);
    u.x /= u.w;
    u.y /= u.w;
    u.z /= u.w;
    return u;
}

#define mat4_print(m)                                                                    \
(                                                                                        \
    printf("%s = \n"                                                                     \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                        \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                        \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                        \
           "    |%14.5f %14.5f %14.5f %14.5f |\n", #m ,                                  \
            (double) (m).c0.x, (double) (m).c1.x, (double) (m).c2.x, (double) (m).c3.x,  \
            (double) (m).c0.y, (double) (m).c1.y, (double) (m).c2.y, (double) (m).c3.y,  \
            (double) (m).c0.z, (double) (m).c1.z, (double) (m).c2.z, (double) (m).c3.z,  \
            (double) (m).c0.w, (double) (m).c1.w, (double) (m).c2.w, (double) (m).c3.w)  \
)

/* ========== scalar interpolation functions =================================*/

static HGL_INLINE float lerp(float a, float b, float amount)
{
    return (1.0f - amount) * a + amount * b; // value
}

static HGL_INLINE float ilerp(float a, float b, float value)
{
    return (value - a) / (b - a); // amount
}

static HGL_INLINE float clamp(float min, float max, float value)
{
    //return fminf(fmaxf(min, value), max);
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static HGL_INLINE float remap(float in_min, float in_max, float out_min, float out_max, float value)
{
    float t = ilerp(in_min, in_max, value);
    return lerp(out_min, out_max, t);
}

static HGL_INLINE float smoothstep(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

static HGL_INLINE float sinstep(float t)
{
    return -0.5f * cosf(t * PI) + 0.5f;
}

static HGL_INLINE float lerpsmooth(float a, float b, float dt, float λ)
{
    return b + (a - b) * exp2f(-dt/λ);
}

#endif /* HGL_MATH_H */

