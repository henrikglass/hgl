#define _DEFAULT_SOURCE

#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "raylib.h"
#include "stb_image.h"

#include "rita_helpers.h"

int main()
{
    const int WIDTH = 800;
    const int HEIGHT = 600;

    /* Initialize hgl_rita */
    hgl_rita_init();

    /* Create a framebuffer texture and bind it (depth buffer is optional) */
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Specify the viewport (maps NDC:s to x \in [0,800], y \in [0,600]) */
    hgl_rita_use_viewport(WIDTH, HEIGHT);

    /* Disable depth buffer (since we didn't provide one) */
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);
    hgl_rita_disable(HGL_RITA_DEPTH_TESTING);

    /* Disable backface culling, since we don't care about winding order */
    hgl_rita_disable(HGL_RITA_BACKFACE_CULLING);

    /* Use array-style vertex buffer mode (as opposed to indexed, which would need an index buffer) */
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_ARRAY);

    /* Create a vertex buffer and fill it with the vertices of a quad. */
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);
    HglRitaVertexBuffer vbuf = {};
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.5, .pos.y = -0.5, .uv.x = 0.0f, .uv.y = 1.0f, .color = HGL_RITA_WHITE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.5, .pos.y = -0.5, .uv.x = 1.0f, .uv.y = 1.0f, .color = HGL_RITA_WHITE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.5, .pos.y =  0.5, .uv.x = 0.0f, .uv.y = 0.0f, .color = HGL_RITA_WHITE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.5, .pos.y =  0.5, .uv.x = 1.0f, .uv.y = 0.0f, .color = HGL_RITA_WHITE});

    /* Bind the vertex buffer to the hgl_rita context */
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf);

    /* Load texture and bind it to the default texture unit */
    HglRitaTexture box_texture = load_texture("assets/box64x64.png");
    hgl_rita_bind_texture(HGL_RITA_TEX_DIFFUSE, &box_texture); // Aka HGL_RITA_TEX_DEFAULT, aka 0.

    /* Load another texture and blit it onto the center of the box texture (using bilinear filtering for some reason :3) */
    HglRitaTexture duck_texture = load_texture("assets/duck.png");
    hgl_rita_use_texture_filter(HGL_RITA_BILINEAR);
    hgl_rita_texture_blit(hgl_rita_texture_get_subtexture(box_texture, 16, 16, 32, 32), // destination
                          duck_texture,                                                 // source
                          HGL_RITA_ALPHA,                                               // blend method
                          true);                                                        // flip vertically
    hgl_rita_use_texture_filter(HGL_RITA_NEAREST);

    /* Load third texture */
    HglRitaTexture rainbow_texture = load_texture("assets/rainbow_gradient.png");

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

    SetTargetFPS(60);
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR);
        hgl_rita_draw(HGL_RITA_TRIANGLE_STRIP);

        /* 
         * blit the third texture to the entire framebuffer, wherever the 
         * color is the same as the background color.
         */
        hgl_rita_blit(0, 0, WIDTH, HEIGHT, 
                      rainbow_texture, 
                      HGL_RITA_REPLACE,
                      HGL_RITA_CLEAR_COLOR,
                      HGL_RITA_SCREENCOORD,
                      NULL);
        hgl_rita_finish();
        
        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTexture(color_tex, 0, 0, WHITE);
        EndDrawing();
    }

    CloseWindow();
}
