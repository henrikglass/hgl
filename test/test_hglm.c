
#include "hgl_test.h"

#define HGLM_STRIP_PREFIX
#include "hglm.h"

bool float_eq(float a, float b)
{
    const float EPSILON = 0.0001f;
    return fabsf(a - b) < EPSILON;
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

bool mat4_eq(Mat4 m0, Mat4 m1)
{
    return vec4_eq(m0.c0, m1.c0) &&
           vec4_eq(m0.c1, m1.c1) &&
           vec4_eq(m0.c2, m1.c2) &&
           vec4_eq(m0.c3, m1.c3);
}

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

