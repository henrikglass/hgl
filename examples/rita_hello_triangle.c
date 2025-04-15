#define _DEFAULT_SOURCE

#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "raylib.h"
#include "stb_image.h"

int main()
{
    const int WIDTH = 576;
    const int HEIGHT = 512;

    /* Initialize hgl_rita */
    hgl_rita_init();
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Create a framebuffer texture and bind it (depth buffer is optional) */
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);

    /* Specify the viewport (maps NDC:s to x \in [0,800], y \in [0,600]) */
    hgl_rita_use_viewport(WIDTH, HEIGHT);

    /* Disable depth buffer (since we didn't provide one) */
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);
    hgl_rita_disable(HGL_RITA_DEPTH_TESTING);

    /* Disable backface culling, since we don't care about winding order */
    hgl_rita_disable(HGL_RITA_BACKFACE_CULLING);

    /* Use array-style vertex buffer mode (as opposed to indexed, which would need an index buffer) */
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_ARRAY);

    /* Create a vertex buffer and fill it with the vertices of a triangle. */
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);
    HglRitaVertexBuffer vbuf = {0};
    hgl_rita_buf_reserve(&vbuf, 16);

#if 1
    // frontfacing
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.0, .pos.y =  0.5, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.5, .pos.y = -0.5, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.5, .pos.y = -0.5, .color = HGL_RITA_GREEN});
#else
    // backfacing
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.0, .pos.y =  0.5, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.5, .pos.y = -0.5, .color = HGL_RITA_GREEN});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.5, .pos.y = -0.5, .color = HGL_RITA_BLUE});
#endif


    /* Bind the vertex buffer to the hgl_rita context */
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf);

    /* Raylib stuff: IGNORE */
    InitWindow(WIDTH, HEIGHT, "HglRita: Hello Triangle!");
    Image color_image = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR);
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        hgl_rita_finish();
       
        //sleep(1);
        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTexture(color_tex, 0, 0, WHITE);
            DrawFPS(10,10);
        EndDrawing();
    }

    hgl_rita_buf_destroy(&vbuf);
    CloseWindow();
}
