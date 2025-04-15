#define _DEFAULT_SOURCE

#define HGL_RITA_IMPLEMENTATION
#define HGL_RITA_SHADERS_IMPLEMENTATION
#include "hgl_rita.h"
#include "hgl_rita_shaders.h"
#include "rita_helpers.h"

#include "raylib.h"
#include "stb_image.h"

static HglRitaTexture image;
static int frame_count = 0;
static int WIDTH;
static int HEIGHT;

static inline uint32_t hash(uint32_t v)
{
    v += ( v << 10u );
    v ^= ( v >>  6u );
    v += ( v <<  3u );
    v ^= ( v >> 11u );
    v += ( v << 15u );
    return v;
}

static inline Vec3 yiq_adjust_colors(Vec3 yiq, float hue, float sat, float bright)
{
    Mat3 M = {
        .m00 = bright, .m01 =          0.0f, .m02 =          0.0f,
        .m10 =   0.0f, .m11 = sat*cosf(hue), .m12 =    -sinf(hue),
        .m20 =   0.0f, .m21 =     sinf(hue), .m22 = sat*cosf(hue),
    };
    return mat3_mul_vec3(M, yiq); 
}

static inline HglRitaColor analog_camera(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    HglRitaColor color = in->color;
    const float abberation_amount = 0.01f;
    const float grain_amount = 0.21;

    /* Chromatic abberation */
    float strength = powf(vec2_distance(in->uv, vec2_make(0.5f, 0.5f)), 1.4142);
    Vec2 uv_r = in->uv;
    Vec2 uv_g = in->uv;
    Vec2 uv_b = in->uv;
    uv_r.x -= abberation_amount * strength;
    uv_b.x += abberation_amount * strength;
    uint8_t r = hgl_rita_sample_uv(&image, uv_r).r;
    uint8_t g = hgl_rita_sample_uv(&image, uv_g).g;
    uint8_t b = hgl_rita_sample_uv(&image, uv_b).b;
    color.r = r;
    color.g = g;
    color.b = b;

    /* grain */
    uint32_t seed = (in->y * WIDTH + in->x) * frame_count;
    int noise = grain_amount * ((int)(hash(seed) & 0xFF) - 0x7F);
    color.r = clamp(0, 255, color.r + noise);
    color.g = clamp(0, 255, color.g + noise);
    color.b = clamp(0, 255, color.b + noise);

    /* color shift */
    Mat3 rgb2yiq = {
        .m00 = 0.2990f, .m01 =  0.5870f, .m02 =  0.1140f,
        .m10 = 0.5959f, .m11 = -0.2746f, .m12 = -0.3213f,
        .m20 = 0.2115f, .m21 = -0.5227f, .m22 =  0.3112f,
    };
    Mat3 yiq2rgb = {
        .m00 = 1.0f, .m01 =  0.956f, .m02 =  0.619f,
        .m10 = 1.0f, .m11 = -0.272f, .m12 = -0.647f,
        .m20 = 1.0f, .m21 = -1.106f, .m22 =  1.703f,
    };
    Vec4 v_rgb = hgl_rita_color_as_vector(color);
    Vec3 v_yiq = mat3_mul_vec3(rgb2yiq, v_rgb.xyz);
    v_yiq = yiq_adjust_colors(v_yiq, -0.24f, 1.5f, 1.3f);
    v_rgb.xyz = mat3_mul_vec3(yiq2rgb, v_yiq);
    v_rgb.x *= 0.96;
    v_rgb.y *= 0.92;
    v_rgb.z *= 0.81;
    color = hgl_rita_color_from_vector(v_rgb);

    return color;
}

static inline HglRitaColor sobel_sharpen(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    /*
     * Applies the sobel kernel:
     *
     *      |  0, -1,  0 |
     *      | -1,  5, -1 |
     *      |  0, -1,  0 |
     */
    HglRitaColor texel;
    int r = 0;
    int g = 0;
    int b = 0;
    float du = 1.0f / (float)image.width;
    float dv = 1.0f / (float)image.height;
    texel = hgl_rita_sample_uv(&image, vec2_make(in->uv.x, in->uv.y - dv));
    r += -1.0f * texel.r;
    g += -1.0f * texel.g;
    b += -1.0f * texel.b;
    texel = hgl_rita_sample_uv(&image, vec2_make(in->uv.x - du, in->uv.y));
    r += -1.0f * texel.r;
    g += -1.0f * texel.g;
    b += -1.0f * texel.b;
    texel = in->color;
    r += 5.0f * texel.r;
    g += 5.0f * texel.g;
    b += 5.0f * texel.b;
    texel = hgl_rita_sample_uv(&image, vec2_make(in->uv.x + du, in->uv.y));
    r += -1.0f * texel.r;
    g += -1.0f * texel.g;
    b += -1.0f * texel.b;
    texel = hgl_rita_sample_uv(&image, vec2_make(in->uv.x, in->uv.y + dv));
    r += -1.0f * texel.r;
    g += -1.0f * texel.g;
    b += -1.0f * texel.b;
    return (HglRitaColor) {
        .r = clamp(0, 255, r),
        .g = clamp(0, 255, g),
        .b = clamp(0, 255, b),
        .a = 255
    };
}

static struct {
    const char *name;
    HglRitaFragShaderFunc func;
} shaders[] = {
    {.name = "NULL",            .func = NULL},
    {.name = "ANALOG_CAMERA",   .func = analog_camera},
    {.name = "MONOCHROME",      .func = HGL_RITA_MONOCHROME},
    {.name = "SOBEL_SHARPEN",   .func = sobel_sharpen},
    {.name = "DITHER_4X4_1BPP", .func = HGL_RITA_DITHER_4X4_1BPP},
    {.name = "DITHER_4X4_2BPP", .func = HGL_RITA_DITHER_4X4_2BPP},
    {.name = "DITHER_4X4_3BPP", .func = HGL_RITA_DITHER_4X4_3BPP},
};

int main()
{
    /* Load a source image */
    image = load_texture("assets/times_square.png");
    hgl_rita_texture_flip_vertically(&image);
    WIDTH = 2 * image.width;
    HEIGHT = 2 * image.height;

    /* Initialize hgl_rita */
    hgl_rita_init();

    /* Create a framebuffer texture and bind it (depth buffer is optional) */
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Raylib stuff: IGNORE */
    InitWindow(WIDTH, HEIGHT, "HglRita: Post processing!");
    Image color_image = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    SetTargetFPS(60);
    int shader_in_use = 0;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR);
        hgl_rita_blit(0, 0, WIDTH, HEIGHT,
                      image,
                      HGL_RITA_REPLACE,
                      HGL_RITA_EVERYWHERE,
                      (shaders[shader_in_use].func != NULL) ? HGL_RITA_SHADER : 
                                                              HGL_RITA_BOXCOORD,
                      shaders[shader_in_use].func);
        hgl_rita_blit(10, 10, 1000, 80, image, 
                      HGL_RITA_MULTIPLY, 
                      HGL_RITA_EVERYWHERE, 
                      HGL_RITA_SHADER, 
                      HGL_RITA_GRAY_SHADER);
        hgl_rita_draw_text(40, 40, 4, HGL_RITA_WHITE, "Press ENTER to cycle between shaders");

        if (IsKeyPressed(KEY_ENTER)) {
            shader_in_use++;
            shader_in_use %= (sizeof(shaders)/sizeof(shaders[0]));
        }

        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTexture(color_tex, 0, 0, WHITE);
            DrawFPS(10,10);
        EndDrawing();

        frame_count++;
    }

    CloseWindow();
}
