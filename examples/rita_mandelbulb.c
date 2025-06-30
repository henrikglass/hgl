#define _DEFAULT_SOURCE

#define HGL_RITA_TILE_SIZE_X 32
#define HGL_RITA_TILE_SIZE_Y 32
#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "raylib.h"

//#include "ffmpeg.h"

#define WIDTH           320 
#define HEIGHT          180
#define DISPLAY_SCALE     4
#define EPSILON           0.001f

static float fractal_power = 1.0f;

static inline float sdf_mandelbulb(Vec3 p, int *iter)
{
    Vec3 z = p;
    float dr = 1.0f;
    float r = 0.0f;

    int i;
    for (i = 0; i < 15; i++) {
        r = vec3_len(z);
        if (r > 2) break;

        float theta = acosf(z.z / r) * fractal_power;
        float phi = atan2f(z.y, z.x) * fractal_power;
        float zr = powf(r, fractal_power);
        dr = powf(r, fractal_power - 1) * fractal_power * dr + 1;

        z = vec3_mul_scalar(vec3_make(sinf(theta) * cosf(phi), sinf(phi) * sinf(theta), cosf(theta)), zr);
        z = vec3_add(z, p);
    }
    
    *iter = i;
    return 0.5f * logf(r) * r / dr;
}

static inline Vec3 calc_view_dir(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    Mat3 iview = mat3_transpose(mat3_make_from_mat4(ctx->tform.view));
    float x = 2.0f * in->uv.x - 1.0f;
    float y = 2.0f * in->uv.y - 1.0f;
    float z = ctx->tform.proj.m11; 
    Vec3 dir = vec3_make(x * ctx->tform.camera.aspect, -y, -z);
    dir = vec3_normalize(dir);
    dir = mat3_mul_vec3(iview, dir);
    return dir; 
}

static Vec3 calc_normal(Vec3 p) {
    int dummy;
    float x = sdf_mandelbulb(vec3_make(p.x + EPSILON, p.y, p.z), &dummy) - sdf_mandelbulb(vec3_make(p.x - EPSILON, p.y, p.z), &dummy);
    float y = sdf_mandelbulb(vec3_make(p.x, p.y + EPSILON, p.z), &dummy) - sdf_mandelbulb(vec3_make(p.x, p.y - EPSILON, p.z), &dummy);
    float z = sdf_mandelbulb(vec3_make(p.x, p.y, p.z + EPSILON), &dummy) - sdf_mandelbulb(vec3_make(p.x, p.y, p.z - EPSILON), &dummy);
    return vec3_normalize(vec3_make(x, y, z));
}

static inline HglRitaColor raymarch(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    HglRitaColor color = HGL_RITA_MORTEL_BLACK;
    
    Vec3 ro = ctx->tform.camera.position;
    Vec3 rd = calc_view_dir(ctx, in);
    Vec3 p = ro;

    int i = 0;
    for (i = 0; i < 256; i++) {

        /* get signed distance to mandelbulb */
        int iter;
        float d = sdf_mandelbulb(p, &iter);

        /* break early if we can */
        if (d > 10) {
            break;
        }

        /* Hit! */
        if (d < EPSILON) {
            Vec3 N = calc_normal(vec3_sub(p, vec3_mul_scalar(rd, EPSILON*2.0f)));
            N = mat3_mul_vec3(ctx->tform.iview, N);
            float l = 0.5f*(vec3_dot(N, vec3_normalize(vec3_make(1,1,-1))) + 1.0f);
            HglRitaColor c0 = hgl_rita_color_mul_scalar(HGL_RITA_MAGENTA, l);
            HglRitaColor c1 = hgl_rita_color_mul_scalar(HGL_RITA_DARK_GREEN, (float)iter / 16.0f);
            color = hgl_rita_color_add(c0, c1);
            break;
        }
    
        /* march ray */
        p = vec3_add(p, vec3_mul_scalar(rd, d));
    }

    /* Do final blend */
    float t = fmaxf(0, (float)i / 50.0f);
    color = hgl_rita_color_mul_scalar(color, t);
    color.a = 255;
    return color;
}

int main()
{
    /* Initialize hgl_rita */
    hgl_rita_init();
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Create a framebuffer texture and bind it (depth buffer is optional) */
    HglRitaTexture fb = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb);

    hgl_rita_use_perspective_proj(3.14/4, (float)WIDTH/(float)HEIGHT, 1.0, 1000);

    /* Raylib stuff: IGNORE */
    InitWindow(DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, "HglRita: Ray-marched Mandelbulb");
    Image color_image = (Image) {
        .data    = fb.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);
    
    //FFMPEG *ffmpeg = ffmpeg_start_rendering(WIDTH, HEIGHT, 30);
    
    SetTargetFPS(60);
    int frame_count = 0;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* animate */
        hgl_rita_use_camera_view(vec3_make(2.5f*sinf(frame_count*0.005f), 0, 2.5f*cosf(frame_count*0.005f)), 
                                 vec3_make(0,0,0), 
                                 vec3_make(0, 1, 0));
        fractal_power = -5 * cosf(frame_count*0.005f) + 6;

        /* Draw! */
        hgl_rita_blit(0, 0, WIDTH, HEIGHT, NULL,
                      HGL_RITA_REPLACE,
                      HGL_RITA_EVERYWHERE,
                      HGL_RITA_SHADER,
                      raymarch);
        hgl_rita_finish();
       
        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, fb.data.rgba8);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
        EndDrawing();

        //ffmpeg_send_frame(ffmpeg, fb.data.rgba8, WIDTH, HEIGHT);

        frame_count++;
        printf("frame = %d\n", frame_count);
        if (frame_count == 1280) break;
    }

    //ffmpeg_end_rendering(ffmpeg);

    CloseWindow();
}
