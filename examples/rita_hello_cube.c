#define _DEFAULT_SOURCE

#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "raylib.h"
#include "stb_image.h"

#define WIDTH          (800)
#define HEIGHT         (600)
#define DISPLAY_SCALE   1.0

int main()
{
    /* Initialize hgl_rita */
    hgl_rita_init();
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Create a framebuffer texture & depth buffer texture and bind them */
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    HglRitaTexture fb_depth = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_R32);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_bind_texture(HGL_RITA_TEX_DEPTH_BUFFER, &fb_depth);


    /* Setup the camera */
    Mat4 view = mat4_look_at(vec3_make(0, 0, 3),                  // camera position
                             vec3_make(0, 0, 0),                  // target position
                             vec3_make(0, 1, 0));                 // Y-axis is up
    Mat4 proj = mat4_make_perspective(3.1415f/4.0f,               // FOV (45 degrees)
                                      (float)WIDTH/(float)HEIGHT, // aspect ratio
                                      2.0f,                       // near clipping plane
                                      1000.0f);                   // far clipping plane
    hgl_rita_use_view_matrix(view);
    hgl_rita_use_proj_matrix(proj);

    /* Specify the viewport (maps NDC:s to x \in [0,800], y \in [0,600]) */
    hgl_rita_use_viewport(WIDTH, HEIGHT);

    /* Enable depth buffer */
    hgl_rita_enable(HGL_RITA_DEPTH_BUFFER_WRITING);
    hgl_rita_enable(HGL_RITA_DEPTH_TESTING);

    /* Enable backface culling */
    hgl_rita_disable(HGL_RITA_BACKFACE_CULLING);

    /* Use indexed vertex buffer mode */
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);

    /* Create a vertex buffer and fill it with the vertices of a cube. */
    HglRitaVertexBuffer vbuf = {0};
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.5, .pos.y = -0.5, .pos.z = -0.5, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.5, .pos.y = -0.5, .pos.z = -0.5, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.5, .pos.y =  0.5, .pos.z = -0.5, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.5, .pos.y =  0.5, .pos.z = -0.5, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.5, .pos.y = -0.5, .pos.z =  0.5, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.5, .pos.y = -0.5, .pos.z =  0.5, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x = -0.5, .pos.y =  0.5, .pos.z =  0.5, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){.pos.x =  0.5, .pos.y =  0.5, .pos.z =  0.5, .color = HGL_RITA_BLUE});

    /* Create an index buffer and fill it indicies such that each edge of the cube is drawn. */
    HglRitaIndexBuffer ibuf = {0};
    // left to right
    hgl_rita_buf_push(&ibuf, 0); hgl_rita_buf_push(&ibuf, 1);
    hgl_rita_buf_push(&ibuf, 2); hgl_rita_buf_push(&ibuf, 3);
    hgl_rita_buf_push(&ibuf, 4); hgl_rita_buf_push(&ibuf, 5);
    hgl_rita_buf_push(&ibuf, 6); hgl_rita_buf_push(&ibuf, 7);
    // front to back
    hgl_rita_buf_push(&ibuf, 4); hgl_rita_buf_push(&ibuf, 0);
    hgl_rita_buf_push(&ibuf, 5); hgl_rita_buf_push(&ibuf, 1);
    hgl_rita_buf_push(&ibuf, 6); hgl_rita_buf_push(&ibuf, 2);
    hgl_rita_buf_push(&ibuf, 7); hgl_rita_buf_push(&ibuf, 3);
    // top to bottom
    hgl_rita_buf_push(&ibuf, 0); hgl_rita_buf_push(&ibuf, 2);
    hgl_rita_buf_push(&ibuf, 1); hgl_rita_buf_push(&ibuf, 3);
    hgl_rita_buf_push(&ibuf, 4); hgl_rita_buf_push(&ibuf, 6);
    hgl_rita_buf_push(&ibuf, 5); hgl_rita_buf_push(&ibuf, 7);

    /* Bind the vertex buffer to the hgl_rita context */
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf);
    hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &ibuf);

    /* Set up a model matrix for our cube */
    Mat4 model = mat4_make_identity();
    hgl_rita_use_model_matrix(model);

    /* Raylib stuff: IGNORE */
    InitWindow(DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, "HglRita: hello cube");
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
        hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
        hgl_rita_draw(HGL_RITA_LINES);
        hgl_rita_finish();
        hgl_rita_finish();
        hgl_rita_finish();
        hgl_rita_finish();
        hgl_rita_finish();

        /* rotate the cube around the Y-axis a tiny bit per frame. */
        model = mat4_rotate(model, 0.01, vec3_normalize(vec3_make(0.3, 1, 0.4)));
        hgl_rita_use_model_matrix(model);
        
        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
        EndDrawing();
    }

    CloseWindow();
}
