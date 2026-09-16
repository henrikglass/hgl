#define _DEFAULT_SOURCE

#include <stdlib.h>
//#define HGL_EARCLIP_ALLOC malloc
#define HGL_EARCHOP_KEEP_DEGENERATE_TRIANGLES
#define HGL_EARCHOP_IMPLEMENTATION
#include "hgl_earchop.h"

#define HGL_RITA_SINGLE_THREAD
#define HGL_RITA_SIMPLEST
#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"


#define HGL_IO_IMPLEMENTATION
#include "hgl_io.h"

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    // polygon boundary defined in CW winding order
    HglRitaVertexBuffer vbuf = {0};
#if 1
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-2.0f,  4.0f}, .color = HGL_RITA_MORTEL_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-1.0f,  1.0f}, .color = HGL_RITA_MORTEL_GREEN});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = { 1.0f,  3.0f}, .color = HGL_RITA_MORTEL_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = { 1.0f,  6.0f}, .color = HGL_RITA_MORTEL_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = { 4.0f, -1.0f}, .color = HGL_RITA_MORTEL_GREEN});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = { 1.0f, -2.0f}, .color = HGL_RITA_MORTEL_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = { 2.0f, -1.0f}, .color = HGL_RITA_MORTEL_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-2.0f, -2.0f}, .color = HGL_RITA_MORTEL_GREEN});
#else
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-4.0f,  1.0f}, .color = HGL_RITA_MORTEL_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-2.0f,  2.0f}, .color = HGL_RITA_MORTEL_GREEN});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-5.0f,  1.0f}, .color = HGL_RITA_MORTEL_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-3.0f, -1.0f}, .color = HGL_RITA_MORTEL_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-3.0f,  0.0f}, .color = HGL_RITA_MORTEL_GREEN});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-2.0f, -2.0f}, .color = HGL_RITA_MORTEL_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-6.0f, -2.0f}, .color = HGL_RITA_MORTEL_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-6.0f,  2.0f}, .color = HGL_RITA_MORTEL_GREEN});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = { 0.0f,  3.0f}, .color = HGL_RITA_MORTEL_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-3.0f,  5.0f}, .color = HGL_RITA_MORTEL_RED});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = { 0.0f,  4.0f}, .color = HGL_RITA_MORTEL_GREEN});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = { 1.0f,  0.0f}, .color = HGL_RITA_MORTEL_BLUE});
    hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-1.0f,  2.0f}, .color = HGL_RITA_MORTEL_RED});
    //hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-4.0f,  1.0f}, .color = HGL_RITA_MORTEL_RED});
    //hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-5.0f,  1.0f}, .color = HGL_RITA_MORTEL_BLUE});
    //hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-3.0f, -1.0f}, .color = HGL_RITA_MORTEL_RED});
    //hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-3.0f,  0.0f}, .color = HGL_RITA_MORTEL_GREEN});
    //hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-2.0f, -2.0f}, .color = HGL_RITA_MORTEL_BLUE});
    //hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-6.0f, -2.0f}, .color = HGL_RITA_MORTEL_RED});
    //hgl_rita_buf_push(&vbuf, (HglRitaVertex){ .pos.xy = {-6.0f,  2.0f}, .color = HGL_RITA_MORTEL_GREEN});
#endif

    int index_buffer_length = 3*hgl_earchop_n_triangles(vbuf.length);
    HglRitaIndexBuffer ibuf = {0};
    hgl_rita_buf_reserve_exact(&ibuf, index_buffer_length);
    ibuf.length = ibuf.capacity;
    int err = hgl_earchop(vbuf.arr,                           // pointer to vertices
                          vbuf.length,                        // # of vertices
                          (int)sizeof(HglRitaVertex),         // vertex stride
                          (int)offsetof(HglRitaVertex, pos),  // offset of xy position information in vertex
                          ibuf.arr,                           // pointer to buffer in which to store indices (can be NULL iff HGL_EARCLIP_ALLOC is defined)
                          &ibuf.length,                       // Pointer to integer in which to store the number of generated triangle indices
                          HGL_EARCHOP_CW,                     // Polygon boundary winding order
                          HGL_EARCHOP_CCW);                   // Winding order of resulting triangles in indices
    assert(err == 0);

    for (int y = 0; y < ibuf.length/3; y++) {
        printf("(%d, %d, %d)\n", ibuf.arr[3*y + 0], ibuf.arr[3*y + 1], ibuf.arr[3*y + 2]);
    }


    hgl_rita_init();
    HglRitaTexture fb_color = hgl_rita_texture_make(512, 512, HGL_RITA_RGBA8);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);
    hgl_rita_use_viewport(512, 512);
    Mat4 view = mat4_make_ortho(-10, 10, -10, 10, -10, 10);
    hgl_rita_use_view_matrix(view);
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);
    hgl_rita_disable(HGL_RITA_DEPTH_TESTING);
    hgl_rita_use_frontface_winding_order(HGL_RITA_CCW);
    hgl_rita_enable(HGL_RITA_BACKFACE_CULLING);
    hgl_rita_enable(HGL_RITA_WIRE_FRAMES);

    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf);
    hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &ibuf);

    InitWindow(2*512, 2*512, "hgl_earchop.h example");
    SetTargetFPS(60);
    Image color_image = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = 512,
        .height  = 512,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    int frame_count = 0;
    bool use_wireframes = false;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        if (frame_count % 60 == 0) use_wireframes = !use_wireframes;
        if (use_wireframes) {
            hgl_rita_enable(HGL_RITA_WIRE_FRAMES);
        } else {
            hgl_rita_disable(HGL_RITA_WIRE_FRAMES);
        }

        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR);
        hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        hgl_rita_finish();

        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, 2.0f, WHITE);
            DrawFPS(10,10);
        EndDrawing();
        frame_count++;
    }

    CloseWindow();
}
