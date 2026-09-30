#define _DEFAULT_SOURCE

#define HGL_RITA_TILE_SIZE_X 256
#define HGL_RITA_TILE_SIZE_Y 32
//#define HGL_RITA_RENDERER_PRESET_SINGLE_THREAD
#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "raylib.h"
#include "stb_image.h"

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
    hgl_rita_enable(HGL_RITA_SHOW_TILE_OUTLINES);

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
    InitWindow(WIDTH*4, HEIGHT*2, "HglRita: Hello Triangle!");
    Image img0 = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Image img = (Image) {
        .data    = fb_downsample.data.rgba8,
        .width   = WIDTH/2,
        .height  = HEIGHT/2,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };

    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR);
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        hgl_rita_finish();

        /* Downsample! */
        hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_downsample);
        hgl_rita_use_texture_filter(HGL_RITA_BILINEAR);
        hgl_rita_blit(0, 0, WIDTH/2, HEIGHT/2, &fb_color, HGL_RITA_REPLACE, HGL_RITA_EVERYWHERE, HGL_RITA_BOXCOORD, NULL);
        hgl_rita_finish();
       
        //sleep(1);
        /* raylib stuff: IGNORE */
        BeginDrawing();
            Texture2D color_tex = LoadTextureFromImage(img0);
            UpdateTexture(color_tex, img0.data);
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, 2.0f, WHITE);
        EndDrawing();
        BeginDrawing();
            color_tex = LoadTextureFromImage(img);
            UpdateTexture(color_tex, img.data);
            DrawTextureEx(color_tex, (Vector2){2*WIDTH, 0}, 0, 4.0f, WHITE);
            DrawFPS(10,10);
        EndDrawing();
    }

    hgl_rita_buf_destroy(&vbuf);
    CloseWindow();
}
