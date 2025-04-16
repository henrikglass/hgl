
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2025 Henrik A. Glass
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
 * hgl_rita_shaders.h contains a collection of ready-to-use shaders for hgl_rita.h.
 *
 * USAGE:
 *
 * See `hgl_rita.h`
 *
 *
 * EXAMPLES:
 *
 * See the examples/ directory.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_RITA_SHADERS_H
#define HGL_RITA_SHADERS_H

/* vertex shaders */
static inline HglRitaVertex HGL_RITA_VERTEX_SNAP_SHADER(const HglRitaContext *ctx, const HglRitaVertex *in);
static inline HglRitaVertex HGL_RITA_VERTEX_DIRECTIONAL_LIGHT_SHADER(const HglRitaContext *ctx, const HglRitaVertex *in);

/* fragment shaders */
static inline HglRitaColor HGL_RITA_UV_SHADER(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_NORMAL_SHADER(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_DEPTH_SHADER(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_LAMBERT_DIFFUSE(const HglRitaContext *ctx, const HglRitaFragment *in);
#ifndef HGL_RITA_SIMPLE
static inline HglRitaColor HGL_RITA_PHONG(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_BLINN_PHONG(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_GOOCH(const HglRitaContext *ctx, const HglRitaFragment *in);
#endif

/* filter/post-processing fragment shaders */
static inline HglRitaColor HGL_RITA_GRAY_SHADER(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_MONOCHROME(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_FOG(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_DEPTH_BASED_BORDERS(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_DITHER_4X4_1BPP(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_DITHER_4X4_2BPP(const HglRitaContext *ctx, const HglRitaFragment *in);
static inline HglRitaColor HGL_RITA_DITHER_4X4_3BPP(const HglRitaContext *ctx, const HglRitaFragment *in);

#endif /* HGL_RITA_SHADERS_H */

#ifdef HGL_RITA_IMPLEMENTATION

static inline HglRitaVertex HGL_RITA_VERTEX_SNAP_SHADER(const HglRitaContext *ctx, const HglRitaVertex *in)
{
    HglRitaVertex out;
    Vec4 v;

    /* get local space vertex pos and extend to 4D */
    v = in->pos;
    v.w = 1.0f;


    /* local space --> world space -> view space */
    v = mat4_mul_vec4(ctx->tform.mv, v);

    v.x = ((int)(7.0f*v.x))/7.0f;
    v.y = ((int)(7.0f*v.y))/7.0f;
    v.z = ((int)(7.0f*v.z))/7.0f;

    /* view space -> clip space */
    v = mat4_mul_vec4(ctx->tform.proj, v);

    out.pos     = v;
    out.normal  = in->normal;
#ifndef HGL_RITA_SIMPLE
    out.tangent = in->tangent;
#endif
    out.uv      = in->uv;
    out.color   = in->color;

    return out;
}

static inline HglRitaVertex HGL_RITA_VERTEX_DIRECTIONAL_LIGHT_SHADER(const HglRitaContext *ctx, const HglRitaVertex *in)
{
    HglRitaVertex out;
    Vec4 v;

    /* get local space vertex pos and extend to 4D */
    v = in->pos;
    v.w = 1.0f;

    /* local space --> world space -> view space -> clip space */
    v = mat4_mul_vec4(ctx->tform.mvp, v);

    out.pos     = v;
    out.normal  = in->normal;
#ifndef HGL_RITA_SIMPLE
    out.tangent = in->tangent;
#endif
    out.uv      = in->uv;
    out.color   = in->color;

    Vec3 n = mat3_mul_vec3(ctx->tform.normals, in->normal);
    float light = 0.2f + 0.8f*clamp(0, 1, vec3_dot(n, vec3_normalize(vec3_make(1,1,1))));
    out.color = hgl_rita_color_mul_scalar(out.color, light);
    out.color.a = 255;

    return out;
}

static inline HglRitaColor HGL_RITA_UV_SHADER(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    HglRitaColor color = (HglRitaColor) {
        .r = in->uv.x * 255,
        .g = in->uv.y * 255,
        .b = 0,
        .a = 255,
    };
    return color;
}

static inline HglRitaColor HGL_RITA_NORMAL_SHADER(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    HglRitaColor color = (HglRitaColor) {
        .r = (in->world_normal.x * 0.5f + 0.5f) * 255,
        .g = (in->world_normal.y * 0.5f + 0.5f) * 255,
        .b = (in->world_normal.z * 0.5f + 0.5f) * 255,
        .a = 255,
    };
    return color;
}

static inline HglRitaColor HGL_RITA_DEPTH_SHADER(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    HglRitaColor color = (HglRitaColor) {
        .r = (1.0f - (1.0f / in->inv_z)) * 255,
        .g = (1.0f - (1.0f / in->inv_z)) * 255,
        .b = (1.0f - (1.0f / in->inv_z)) * 255,
        .a = 255,
    };
    return color;
}

static inline HglRitaColor HGL_RITA_LAMBERT_DIFFUSE(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    HglRitaColor color = in->color;
    if (hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DIFFUSE] != NULL) {
        color = hgl_rita_color_mul(color, hgl_rita_sample_unit_uv(HGL_RITA_TEX_DIFFUSE, in->uv));
    }
    float light = 0.2f + 0.8f*clamp(0, 1, vec3_dot(in->world_normal, vec3_normalize(vec3_make(1,1,1))));
    color.r *= light;
    color.g *= light;
    color.b *= light;
    return color;
}

#ifndef HGL_RITA_SIMPLE
static inline HglRitaColor HGL_RITA_PHONG(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    Vec3 L  = vec3_normalize(vec3_make(1,1,1)); // light vector
    Vec3 IV = vec3_normalize(vec3_sub(in->world_pos, ctx->tform.camera.position)); // inverse view vector
    Vec3 N  = in->world_normal;

    float shinyness = 0.5f;
    float specular_exponent = 25.0f;

    HglRitaColor diffuse_color = in->color;
    HglRitaColor specular_color = HGL_RITA_WHITE;

    diffuse_color = hgl_rita_sample_unit_uv(HGL_RITA_TEX_DIFFUSE, in->uv);
    float diffuse_light = clamp(0.1f, 1.0f, fmaxf(0, vec3_dot(N, L))); // Lambertian
    float specular_light = powf(fmaxf(0, vec3_dot(vec3_reflect(IV, N), L)), specular_exponent); // Phong

    diffuse_color.r *= diffuse_light;
    diffuse_color.g *= diffuse_light;
    diffuse_color.b *= diffuse_light;

    specular_color.r *= shinyness * specular_light;
    specular_color.g *= shinyness * specular_light;
    specular_color.b *= shinyness * specular_light;

    return hgl_rita_color_add(diffuse_color, specular_color);
}

static inline HglRitaColor HGL_RITA_BLINN_PHONG(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    Vec3 L = vec3_normalize(vec3_make(1,1,1));
    Vec3 V = vec3_normalize(vec3_sub(ctx->tform.camera.position, in->world_pos));
    Vec3 N = in->world_normal;
    Vec3 H = vec3_normalize(vec3_add(L, V)); // half vector (Blinn-Phong)

    float shinyness = 0.5f;
    float specular_exponent = 70.0f;

    HglRitaColor diffuse_color = in->color;
    HglRitaColor specular_color = HGL_RITA_WHITE;

    diffuse_color = hgl_rita_sample_unit_uv(HGL_RITA_TEX_DIFFUSE, in->uv);
    float diffuse_light = clamp(0.1f, 1.0f, fmaxf(0, vec3_dot(N, L))); // Lambertian
    float specular_light = powf(fmaxf(0, vec3_dot(H, N)), specular_exponent); // Blinn-Phong

    diffuse_color.r *= diffuse_light;
    diffuse_color.g *= diffuse_light;
    diffuse_color.b *= diffuse_light;

    specular_color.r *= shinyness * specular_light;
    specular_color.g *= shinyness * specular_light;
    specular_color.b *= shinyness * specular_light;

    return hgl_rita_color_add(diffuse_color, specular_color);
}

static inline HglRitaColor HGL_RITA_GOOCH(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    Vec3 L  = vec3_normalize(vec3_make(1,1,1)); // light vector
    Vec3 IV = vec3_normalize(vec3_sub(in->world_pos, ctx->tform.camera.position)); // inverse view vector
    Vec3 N  = in->world_normal;

    HglRitaColor cool_color = {  0,  60, 240, 255};
    HglRitaColor warm_color = {250, 140,  20, 255};
    float diffuse_light = vec3_dot(N, L) * 0.5 + 0.5; // Lambertian
    float specular_light = powf(fmaxf(0, vec3_dot(vec3_reflect(IV, N), L)), 15.0f); // Phong
    return hgl_rita_color_add(hgl_rita_color_lerp(cool_color, warm_color, diffuse_light),
                              hgl_rita_color_mul_scalar(HGL_RITA_WHITE, specular_light));
}
#endif

static inline HglRitaColor HGL_RITA_GRAY_SHADER(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    (void) in;
    return HGL_RITA_DARK_GRAY;
}

static inline HglRitaColor HGL_RITA_MONOCHROME(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    //HglRitaColor color = hgl_rita_sample_unit(HGL_RITA_TEX_FRAME_BUFFER, in->x, in->y);
    HglRitaColor color = in->color;
    float luminance = hgl_rita_color_luminance(color);
    return (HglRitaColor) {
        .r = luminance * 255,
        .g = luminance * 255,
        .b = luminance * 255,
        .a = 255,
    };
}

static inline HglRitaColor HGL_RITA_FOG(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    return hgl_rita_color_lerp(in->color, HGL_RITA_LIGHT_GRAY, powf(in->inv_z, 40));
}

static inline HglRitaColor HGL_RITA_DEPTH_BASED_BORDERS(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    int spread = 2;
    int threshold = 2;
    int depth = 255*in->inv_z; 
    int neighbour_depth;
    HglRitaColor border_color = hgl_rita_color_sub(HGL_RITA_WHITE, ctx->opts.clear_color); 
    border_color.a = 255;

    neighbour_depth = hgl_rita_sample_unit(HGL_RITA_TEX_DEPTH_BUFFER, in->x, in->y + spread).r; 
    if (abs(depth - neighbour_depth) > threshold) return border_color;
    neighbour_depth = hgl_rita_sample_unit(HGL_RITA_TEX_DEPTH_BUFFER, in->x + spread, in->y).r; 
    if (abs(depth - neighbour_depth) > threshold) return border_color;
#if 0
    neighbour_depth = hgl_rita_sample_unit(HGL_RITA_TEX_DEPTH_BUFFER, in->x, in->y - spread).r; 
    if (abs(depth - neighbour_depth) > threshold) return border_color;
    neighbour_depth = hgl_rita_sample_unit(HGL_RITA_TEX_DEPTH_BUFFER, in->x - spread, in->y).r; 
    if (abs(depth - neighbour_depth) > threshold) return border_color;
#endif

    return in->color;
}

static inline HglRitaColor HGL_RITA_DITHER_4X4_1BPP(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    float color_depth = 1.0f;
    float spread      = 1.0f;
    float bias        = -0.1f;
    int n_colors = powf(2, color_depth);
    const float bayer4x4[4][4] = {
        {0,   8,  2, 10},
        {12,  4, 14,  6},
        {3,  11,  1,  9},
        {15,  7, 13,  5},
    };
    HglRitaColor color = in->color;
    float M = (1.0f/16.0f)*bayer4x4[in->y & 3][in->x & 3] - 0.5f;
    float r = clamp(0, 1, color.r/255.0f + spread*M + bias);
    float g = clamp(0, 1, color.g/255.0f + spread*M + bias);
    float b = clamp(0, 1, color.b/255.0f + spread*M + bias);
    r = floorf(r * (n_colors - 1) + 0.5f) / (n_colors - 1);
    g = floorf(g * (n_colors - 1) + 0.5f) / (n_colors - 1);
    b = floorf(b * (n_colors - 1) + 0.5f) / (n_colors - 1);
    return (HglRitaColor) {
        .r = 255 * r,
        .g = 255 * g,
        .b = 255 * b,
        .a = 255,
    };
}

static inline HglRitaColor HGL_RITA_DITHER_4X4_2BPP(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    float color_depth = 2.0f;
    float spread      = 0.7f;
    float bias        = -0.2f;
    int n_colors = powf(2, color_depth);
    const float bayer4x4[4][4] = {
        {0,   8,  2, 10},
        {12,  4, 14,  6},
        {3,  11,  1,  9},
        {15,  7, 13,  5},
    };
    HglRitaColor color = in->color;
    float M = (1.0f/16.0f)*bayer4x4[in->y & 3][in->x & 3] - 0.5f;
    float r = clamp(0, 1, color.r/255.0f + spread*M + bias);
    float g = clamp(0, 1, color.g/255.0f + spread*M + bias);
    float b = clamp(0, 1, color.b/255.0f + spread*M + bias);
    r = floorf(r * (n_colors - 1) + 0.5f) / (n_colors - 1);
    g = floorf(g * (n_colors - 1) + 0.5f) / (n_colors - 1);
    b = floorf(b * (n_colors - 1) + 0.5f) / (n_colors - 1);
    return (HglRitaColor) {
        .r = 255 * r,
        .g = 255 * g,
        .b = 255 * b,
        .a = 255,
    };
}

static inline HglRitaColor HGL_RITA_DITHER_4X4_3BPP(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    float color_depth = 3.0f;
    float spread      = 0.7f;
    float bias        = -0.2f;
    int n_colors = powf(2, color_depth);
    const float bayer4x4[4][4] = {
        {0,   8,  2, 10},
        {12,  4, 14,  6},
        {3,  11,  1,  9},
        {15,  7, 13,  5},
    };
    HglRitaColor color = in->color;
    float M = (1.0f/16.0f)*bayer4x4[in->y & 3][in->x & 3] - 0.5f;
    float r = clamp(0, 1, color.r/255.0f + spread*M + bias);
    float g = clamp(0, 1, color.g/255.0f + spread*M + bias);
    float b = clamp(0, 1, color.b/255.0f + spread*M + bias);
    r = floorf(r * (n_colors - 1) + 0.5f) / (n_colors - 1);
    g = floorf(g * (n_colors - 1) + 0.5f) / (n_colors - 1);
    b = floorf(b * (n_colors - 1) + 0.5f) / (n_colors - 1);
    return (HglRitaColor) {
        .r = 255 * r,
        .g = 255 * g,
        .b = 255 * b,
        .a = 255,
    };
}

#endif
