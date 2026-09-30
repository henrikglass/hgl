#define _DEFAULT_SOURCE

//#define HGL_RITA_TILE_SIZE_X 256
//#define HGL_RITA_TILE_SIZE_Y 128
#define HGL_RITA_RENDERER_PRESET_SINGLE_THREAD
#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "raylib.h"
#include "stb_image.h"

static inline HglRitaColor my_shader(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    Vec2 s = vec2(1.0f / ctx->tex_unit[HGL_RITA_TEX_DEFAULT]->width,
                  1.0f / ctx->tex_unit[HGL_RITA_TEX_DEFAULT]->height);
    Vec2 rgss[] = {
        vec2_add(in->uv, vec2_hadamard(vec2( 0.75f,  0.25f), s)),
        vec2_add(in->uv, vec2_hadamard(vec2(-0.25f,  0.75f), s)),
        vec2_add(in->uv, vec2_hadamard(vec2(-0.75f, -0.25f), s)),
        vec2_add(in->uv, vec2_hadamard(vec2( 0.25f, -0.75f), s)),
    };
    //Vec2 rgss[] = {
    //    vec2_add(in->uv, vec2_hadamard(vec2( 0.5,  0.5), s)),
    //    vec2_add(in->uv, vec2_hadamard(vec2(-0.5,  0.5), s)),
    //    vec2_add(in->uv, vec2_hadamard(vec2(-0.5, -0.5), s)),
    //    vec2_add(in->uv, vec2_hadamard(vec2( 0.5, -0.5), s)),
    //};
    HglRitaColor s0 = hgl_rita_sample_unit_uv(HGL_RITA_TEX_DEFAULT, rgss[0]);
    HglRitaColor s1 = hgl_rita_sample_unit_uv(HGL_RITA_TEX_DEFAULT, rgss[1]);
    HglRitaColor s2 = hgl_rita_sample_unit_uv(HGL_RITA_TEX_DEFAULT, rgss[2]);
    HglRitaColor s3 = hgl_rita_sample_unit_uv(HGL_RITA_TEX_DEFAULT, rgss[3]);
    float r, g, b, a;
    r = (float)(s0.r + s1.r + s2.r + s3.r) / 4.0f;
    g = (float)(s0.g + s1.g + s2.g + s3.g) / 4.0f;
    b = (float)(s0.b + s1.b + s2.b + s3.b) / 4.0f;
    a = 255;
    return (HglRitaColor) {
        .r = (uint8_t)r,
        .g = (uint8_t)g,
        .b = (uint8_t)b,
        .a = a,
    };
}

int main()
{
    const int WIDTH = 512;
    const int HEIGHT = 512;

    /* Initialize hgl_rita */
    hgl_rita_init();
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Create a framebuffer texture and bind it (depth buffer is optional) */
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);

    /* Create a framebuffer texture for the downsampling blit */
    HglRitaTexture fb_downsample = hgl_rita_texture_make(WIDTH/2, HEIGHT/2, HGL_RITA_RGBA8);

    /* Specify the viewport (maps NDC:s to x \in [0,800], y \in [0,600]) */
    hgl_rita_use_viewport(WIDTH, HEIGHT);

    /* Disable depth buffer (since we didn't provide one) */
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);
    hgl_rita_disable(HGL_RITA_DEPTH_TESTING);
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);

    /* Disable backface culling, since we don't care about winding order */
    hgl_rita_disable(HGL_RITA_BACKFACE_CULLING);

    /* Use array-style vertex buffer mode (as opposed to indexed, which would need an index buffer) */
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_ARRAY);

    /* Create a vertex buffer and fill it with the vertices of a triangle. */
    HglRitaVertexBuffer vbuf = {0};
    hgl_rita_buf_reserve(&vbuf, 16);

    // frontfacing
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.0, .pos.y =  0.9, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.7, .pos.y = -0.29, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.4, .pos.y = -0.45, .color = HGL_RITA_GREEN});

    /* Bind the vertex buffer to the hgl_rita context */
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf);

    /* Raylib stuff: IGNORE */
    InitWindow(WIDTH*2, HEIGHT*1, "HglRita: Hello Triangle!");
    Image img0 = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D tex0 = LoadTextureFromImage(img0);
    Image img1 = (Image) {
        .data    = fb_downsample.data.rgba8,
        .width   = WIDTH/2,
        .height  = HEIGHT/2,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D tex1 = LoadTextureFromImage(img1);

    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR);
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        hgl_rita_finish();

        /* Downsample! */
        hgl_rita_bind_texture(HGL_RITA_TEX_DEFAULT, &fb_color);
        hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_downsample);
        hgl_rita_use_texture_filter(HGL_RITA_NEAREST);
        hgl_rita_blit(0, 0, WIDTH/2, HEIGHT/2, &fb_color, HGL_RITA_REPLACE, HGL_RITA_EVERYWHERE, HGL_RITA_SHADER, my_shader);
        //hgl_rita_use_texture_filter(HGL_RITA_BILINEAR);
        //hgl_rita_blit(0, 0, WIDTH/2, HEIGHT/2, &fb_color, HGL_RITA_REPLACE, HGL_RITA_EVERYWHERE, HGL_RITA_BOXCOORD, NULL);
        hgl_rita_finish();
       
        /* raylib stuff: IGNORE */
        BeginDrawing();
            UpdateTexture(tex0, img0.data);
            DrawTextureEx(tex0, (Vector2){0, 0}, 0, 1.0f, WHITE);
        EndDrawing();
        BeginDrawing();
            UpdateTexture(tex1, img1.data);
            DrawTextureEx(tex1, (Vector2){WIDTH, 0}, 0, 2.0f, WHITE);
            DrawFPS(10,10);
        EndDrawing();
    }

    hgl_rita_buf_destroy(&vbuf);
    CloseWindow();
}
