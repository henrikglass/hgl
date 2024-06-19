#include <stdio.h>

#include "hgl_math.h"


int main()
{
#ifdef HGL_MATH_USE_SIMD
    printf("Using simd.\n");
#else
    printf("Not using simd.\n");
#endif

    //printf("Hello!\n");
    //printf("lerp(5.0f, 20.0f, 0.0f) = %f\n", lerp(5.0f, 20.0f, 0.0f));
    //printf("lerp(5.0f, 20.0f, 0.5f) = %f\n", lerp(5.0f, 20.0f, 0.5f));
    //printf("lerp(5.0f, 20.0f, 0.6f) = %f\n", lerp(5.0f, 20.0f, 0.6f));
    //printf("lerp(5.0f, 20.0f, 1.0f) = %f\n", lerp(5.0f, 20.0f, 1.0f));
    //printf("lerp(5.0f, 20.0f, 10.0f) = %f\n", lerp(5.0f, 20.0f, 10.0f));
    //printf("clamp(lerp(5.0f, 20.0f, 10.0f)) = %f\n", clamp(5.0f, 20.0f, lerp(5.0f, 20.0f, 10.0f)));
    //printf("remap 5 \\in [0, 10] --> [-50, 50] = %f\n", remap(0, 10, -50, 50, 5));
    //printf("smoothstep(0.0) = %f\n", smoothstep(0.0f));
    //printf("smoothstep(0.1) = %f\n", smoothstep(0.1f));
    //printf("smoothstep(0.2) = %f\n", smoothstep(0.2f));
    //printf("smoothstep(1.0) = %f\n", smoothstep(1.0f));
    //printf("smoothstep(1.2) = %f\n", smoothstep(1.2f));
    //printf("sinstep(0.0) = %f\n", sinstep(0.0f));
    //printf("sinstep(0.1) = %f\n", sinstep(0.1f));
    //printf("sinstep(0.2) = %f\n", sinstep(0.2f));
    //printf("sinstep(1.0) = %f\n", sinstep(1.0f));
    //printf("sinstep(1.2) = %f\n", sinstep(1.2f));
    //printf("smooth [5,20] for 0.0 = %f\n", lerp(5, 20, sinstep(clamp(0.0f, 1.0f, 0.0f))));
    //printf("smooth [5,20] for 0.1 = %f\n", lerp(5, 20, sinstep(clamp(0.0f, 1.0f, 0.1f))));
    //printf("smooth [5,20] for 0.2 = %f\n", lerp(5, 20, sinstep(clamp(0.0f, 1.0f, 0.2f))));
    //printf("smooth [5,20] for 1.0 = %f\n", lerp(5, 20, sinstep(clamp(0.0f, 1.0f, 1.0f))));
    //printf("smooth [5,20] for 1.2 = %f\n", lerp(5, 20, sinstep(clamp(0.0f, 1.0f, 1.2f))));
    //float a = 0.0f;
    //float time = 0.0f;
    //while (a < 9.8) {
    //    a = lerpsmooth(0, 10, time, 0.5f);
    //    time += 0.05f;
    //    printf("a = lerpsmooth(a, 2, 0.05f, 0.5f) = %f = ", a); for(int i = 0; i < a*2; i++) printf("*"); printf("\n");
    //}


    //mat4 t = mat4_make_identity();
    //t = mat4_translate(t, vec3_make(100.0f, 100.0f, 0.f));

    //mat4 r = mat4_make_rotation(PI/2.0f, vec3_make(0.0f, 0.0f, 1.0f));

    //mat4 transform = mat4_matmul4(r, t);
    //mat4_print(transform);

    //vec4 v = vec4_make(10,10,0,1);
    //v = mat4_mul_vec4(transform, v);
    //vec3_print(v.xyz);


    //vec2 v2 = vec2_make(1, -1);
    //vec2 n = vec2_make(0, 1);
    //v2 = vec2_reflect(v2, vec2_normalize(n));
    //vec2_print(v2);

    //mat4 pproj = mat4_make_perspective(PI/2, 16.0/9.0, 0, 2000);
    ////pproj = mat4_translate(pproj, vec3_make(10, -10, 0));
    //mat4_print(pproj);
    //pproj = mat4_mul_scalar(pproj, 2.0f);
    //mat4_print(pproj);
    //pproj = mat4_mul_scalar(pproj, 0.5f);
    //mat4_print(pproj);

    //vec4 v4 = (vec4){.xy = v2, .z = 10, .w = 0};
    //vec4_print(v4);
    //v4 = mat4_perspective_project(pproj, v4);
    //vec4_print(v4);

    //printf("distance = %f\n", vec4_distance(vec4_make(1,1,0,0), vec4_make(-1,-1,0,0)));
    //printf("len = %f\n", vec4_len(vec4_make(1,1,0,0)));
    //vec4 normalized = vec4_normalize(vec4_make(1,1,0,0));
    //vec4_print(normalized);

    //printf("dot = %f\n", vec4_dot(vec4_make(1,0,0,0), vec4_make(1,0,0,0)));
    //printf("dot = %f\n", vec4_dot(vec4_make(1,0,0,0), vec4_make(0,0,0,0)));
    //printf("dot = %f\n", vec4_dot(vec4_make(1,0,0,0), vec4_make(-1,0,0,0)));
    //vec4_print(vec4_mul_scalar(vec4_make(1,0,-1,0), 3));


    vec4 vn = vec4_make(1,0,0,0);
    vec4_print(vn);
    mat4 r2 = mat4_make_rotation(0.1, vec3_make(0,0,1));
    for (size_t i = 0; i < 1000000000llu; i++) {
        vn = mat4_mul_vec4(r2, vn);
    }
    vec4_print(vn);


    //mat4 mm = mat4_make_zero();
    //mm.m00 =  1.0f;
    //mm.m10 =  2.0f;
    //mm.m20 =  3.0f;
    //mm.m30 =  4.0f;
    //mm.m01 =  5.0f;
    //mm.m11 =  6.0f;
    //mm.m21 =  7.0f;
    //mm.m31 =  8.0f;
    //mm.m02 =  9.0f;
    //mm.m12 = 10.0f;
    //mm.m22 = 11.0f;
    //mm.m32 = 12.0f;
    //mm.m03 = 13.0f;
    //mm.m13 = 14.0f;
    //mm.m23 = 15.0f;
    //mm.m33 = 16.0f;
    //mat4_print(mm);
    //mm = mat4_transpose(mm);
    //mat4_print(mm);
    //mm = mat4_transpose(mm);
    //mat4_print(mm);
    //mm = mat4_transpose(mm);
    
    //mm = mat4_transpose(mm);
    //printf("pre-mult:\n");
    //mat4_print(mm);

    //printf("mult:\n");
    //mm = mat4_matmul4(MAT4_IDENTITY, mm);
    //mat4_print(mm);

    //mat4 id = mat4_make_identity();
    //id.c2.z = 0.0001;
    //for (size_t i = 0; i < 1000000000llu; i++) {
    //    id = mat4_matmul4(id, id);
    //}
    //mat4_print(id);

    //mat4 r = mat4_make_rotation(0.1, vec3_make(0,0,1));
    //r = mat4_matmul4(r, r);
    //mat4_print(r);

    //printf("%p\n", &id);
    //printf("%p\n", &v4);

    //vec4_print(mat4_mul_vec4(mat4_make_rotation(0.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(1.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(2.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(4.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(8.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(16.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(32.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(128.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(256.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(512.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(1024.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(2048.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(4096.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(8192.0*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(8192.0f*(PI/8), vec3_make(0,0,1)), vec4_make(1,0,0,0)));
    //vec4_print(mat4_mul_vec4(mat4_make_rotation(8192.0*(PI), vec3_make(0,0,1)), vec4_make(1,0,0,0)));

}
