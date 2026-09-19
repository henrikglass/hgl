
#include "hgl_test.h"

#define HGLM_STRIP_PREFIX
#include "hglm.h"

bool float_eq(float a, float b)
{
    const float EPSILON = 0.0001f;
    return fabsf(a - b) < EPSILON;
}

bool float_neq(float a, float b)
{
    const float EPSILON = 0.0001f;
    return fabsf(a - b) > EPSILON;
}

bool vec2_eq(Vec2 v, Vec2 u)
{
    return float_eq(v.x, u.x) &&
           float_eq(v.y, u.y);
}

bool vec3_eq(Vec3 v, Vec3 u)
{
    return vec2_eq(v.xy, u.xy) &&
           float_eq(v.z, u.z);
}

bool vec4_eq(Vec4 v, Vec4 u)
{
    return vec3_eq(v.xyz, u.xyz) &&
           float_eq(v.w, u.w);
}

bool mat2_eq(Mat2 m0, Mat2 m1)
{
    return vec2_eq(m0.c0, m1.c0) &&
           vec2_eq(m0.c1, m1.c1);
}

bool mat3_eq(Mat3 m0, Mat3 m1)
{
    return vec3_eq(m0.c0, m1.c0) &&
           vec3_eq(m0.c1, m1.c1) &&
           vec3_eq(m0.c2, m1.c2);
}

bool mat4_eq(Mat4 m0, Mat4 m1)
{
    return vec4_eq(m0.c0, m1.c0) &&
           vec4_eq(m0.c1, m1.c1) &&
           vec4_eq(m0.c2, m1.c2) &&
           vec4_eq(m0.c3, m1.c3);
}

#define mat4_log(m)                                                                      \
(                                                                                        \
    fprintf(stderr, "%s = \n"                                                            \
            "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
            "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
            "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
            "    |%14.5f %14.5f %14.5f %14.5f |\n", #m ,                                 \
             (double) (m).c0.x, (double) (m).c1.x, (double) (m).c2.x, (double) (m).c3.x, \
             (double) (m).c0.y, (double) (m).c1.y, (double) (m).c2.y, (double) (m).c3.y, \
             (double) (m).c0.z, (double) (m).c1.z, (double) (m).c2.z, (double) (m).c3.z, \
             (double) (m).c0.w, (double) (m).c1.w, (double) (m).c2.w, (double) (m).c3.w) \
)

TEST(test_pid) 
{
    float s_pos = 0;
    float s_vel = 0;
    float target_height = 50;

    const float max_thrust = 40.0f;
    const float g = -9.81f;
    const float dt = 0.01667f;

    float last_error = 0;
    float integral = 0;

    for (int i = 0; i < 5000; i++) {
        /* acc. due to gravity */
        float acc = g;

        /* acc. due to engine thrust */
        float error = target_height - s_pos;
        float thrust = hglm_pid(error, last_error, &integral, 0.45, 0.3, 1.4, dt);
        thrust = clamp(-max_thrust/2, max_thrust/2, thrust) + max_thrust/2;
        last_error = error;

        acc += thrust;

        /* physics.. */
        s_vel += acc*dt;
        s_pos += s_vel*dt;

        /* bounce on ground */
        if (s_pos < 0) {
            s_pos = -s_pos;
            s_vel = -s_vel*0.7f;
        }
    }

    ASSERT(fabsf(target_height - s_pos) < 0.1f);
}

TEST(test_vec2)
{
    Vec2 v = vec2_make(10, 20);
    Vec2 u = vec2_make(5, 5);
    Vec2 t = vec2_make(5, -5);
    ASSERT(vec2_eq(hglm_vec2_add(v, u), vec2_make(15, 25)));
    ASSERT(vec2_eq(hglm_vec2_sub(v, u), vec2_make(5, 15)));
    ASSERT(float_eq(hglm_vec2_distance(v, u), sqrtf(5*5+15*15)));
    ASSERT(float_eq(hglm_vec2_len(v), sqrtf(10*10+20*20)));
    ASSERT(vec2_eq(hglm_vec2_normalize(u), vec2_make(1/sqrtf(2), 1/sqrtf(2))));
    ASSERT(float_eq(hglm_vec2_dot(u, t), 0.0f));
    ASSERT(float_eq(hglm_vec2_dot(u, u), 50.0f));
    ASSERT(vec2_eq(hglm_vec2_hadamard(v, u), vec2_make(50, 100)));
    ASSERT(vec2_eq(hglm_vec2_mul_scalar(v, 0.5f), vec2_make(5, 10)));
    ASSERT(vec2_eq(hglm_vec2_reflect(t, vec2_make(0, 1)), u));
    ASSERT(vec2_eq(hglm_vec2_lerp(u, t, 0.5f), vec2_make(5, 0)));
    ASSERT(vec2_eq(hglm_vec2_bezier3(vec2_make(0,0), 
                                     vec2_make(0,-1),
                                     vec2_make(1,1),
                                     vec2_make(1,0), 0.0f), vec2_make(0, 0)));
    ASSERT(float_eq(hglm_vec2_bezier3(vec2_make(0,0), 
                                     vec2_make(0,-1),
                                     vec2_make(1,1),
                                     vec2_make(1,0), 0.5f).x, 0.5f));
    ASSERT(vec2_eq(hglm_vec2_bezier3(vec2_make(0,0), 
                                     vec2_make(0,-1),
                                     vec2_make(1,1),
                                     vec2_make(1,0), 1.0f), vec2_make(1, 0)));
}

TEST(test_ortho_inv)
{
    Mat4 M;
    Mat4 m = mat4_make_ortho(-5, 10, -10, 15, -2, 4);
    Mat4 m_inv = mat4_make_ortho_inverse(-5, 10, -10, 15, -2, 4);

    M = mat4_mul_mat4(m, m_inv);
    ASSERT(mat4_eq(M, MAT4_IDENTITY));
    M = mat4_mul_mat4(m_inv, m);
    ASSERT(mat4_eq(M, MAT4_IDENTITY));
}

TEST(test_determinants)
{
    Mat2 m2 = mat2_make_identity();
    ASSERT(float_neq(mat2_det(m2), 0.0f));

    Mat3 m3 = mat3_make_identity();
    ASSERT(float_neq(mat3_det(m3), 0.0f));
}

TEST(test_inverse_matrices)
{
    Mat2 m2 = mat2_make_identity();
    Mat2 m2_inv = mat2_make_inverse(m2);

    ASSERT(mat2_eq(m2, m2_inv));

    Mat3 m3 = mat3_make_identity();
    Mat3 m3_inv = mat3_make_inverse(m3);

    ASSERT(mat3_eq(m3, m3_inv));

    m2 = (Mat2) {
        .c0 = {.x = 2, .y =  1},
        .c1 = {.x = 2, .y = -1},
    };
    m2_inv = mat2_make_inverse(m2);
    ASSERT(float_neq(mat2_det(m2), 0.0f));
    ASSERT(mat2_eq(mat2_mul_mat2(m2, m2_inv), MAT2_IDENTITY));

    m3 = (Mat3) {
        .c0 = {.x = 2, .y =  1, .z =  3},
        .c1 = {.x = 2, .y = -1, .z =  7},
        .c2 = {.x = 4, .y = 12, .z = 13},
    };
    m3_inv = mat3_make_inverse(m3);
    ASSERT(float_neq(mat3_det(m3), 0.0f));
    ASSERT(mat3_eq(mat3_mul_mat3(m3, m3_inv), MAT3_IDENTITY));

    Mat4 m4 = mat4_make_identity();
    m4 = mat4_translate(m4, vec3_make(3, 5, 7));
    m4 = mat4_rotate(m4, 3.14f/5.0f, vec3_normalize(vec3_make(0, 1, 1)));
    m4 = mat4_scale(m4, vec3_make(2, 3, 4));
    mat4_log(m4);
    Mat4 m4_inv = mat4_make_affine_inverse(m4);
    mat4_log(m4_inv);
    Mat4 p4 = mat4_mul_mat4(m4, m4_inv);
    mat4_log(p4);
    ASSERT(mat4_eq(p4, MAT4_IDENTITY));
}
