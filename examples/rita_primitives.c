#define _DEFAULT_SOURCE

//#define HGL_RITA_PARALLEL_VERTEX_PROCESSING
#define HGL_RITA_IMPLEMENTATION
//#include "hgl_rita.h"
#include "hgl_rita.h"

#include "raylib.h"
#include "stb_image.h"

#define WIDTH          (1920/4)
#define HEIGHT         (1080/4)
#define DISPLAY_SCALE      4.0

int main()
{
    /* Create a framebuffer texture and bind it (depth buffer is optional) */
    hgl_rita_init();
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);
    hgl_rita_use_viewport(WIDTH, HEIGHT);
    Mat4 view = mat4_make_ortho(0, DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, 0, -10, 10);
    hgl_rita_use_view_matrix(view);
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);
    hgl_rita_disable(HGL_RITA_DEPTH_TESTING);
    hgl_rita_enable(HGL_RITA_BACKFACE_CULLING);
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_ARRAY);


    /* vertex buffer: Triangles */
    HglRitaVertexBuffer vbuf_tris = {0};
    hgl_rita_buf_push(&vbuf_tris, (HglRitaVertex){.pos.x =  50, .pos.y =  50, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf_tris, (HglRitaVertex){.pos.x =  30, .pos.y = 100, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf_tris, (HglRitaVertex){.pos.x = 200, .pos.y = 200, .color = HGL_RITA_GREEN});
    hgl_rita_buf_push(&vbuf_tris, (HglRitaVertex){.pos.x = 100 +  50, .pos.y =  50, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf_tris, (HglRitaVertex){.pos.x = 100 +  30, .pos.y = 100, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf_tris, (HglRitaVertex){.pos.x = 100 + 200, .pos.y = 200, .color = HGL_RITA_GREEN});

    /* vertex buffer: Lines */
    HglRitaVertexBuffer vbuf_lines = {0};
    hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 400, .pos.y =  50, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 420, .pos.y =  60, .color = HGL_RITA_GREEN});
    hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 440, .pos.y =  80, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 460, .pos.y = 120, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 480, .pos.y = 200, .color = HGL_RITA_GREEN});
    hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 500, .pos.y = 360, .color = HGL_RITA_BLUE});

    /* vertex buffer: Points */
    HglRitaVertexBuffer vbuf_points = {0};
    hgl_rita_buf_push(&vbuf_points, (HglRitaVertex){.pos.x = 600, .pos.y =  40, .color = HGL_RITA_WHITE});
    hgl_rita_buf_push(&vbuf_points, (HglRitaVertex){.pos.x = 600, .pos.y =  60, .color = HGL_RITA_WHITE});
    hgl_rita_buf_push(&vbuf_points, (HglRitaVertex){.pos.x = 600, .pos.y =  80, .color = HGL_RITA_WHITE});
    hgl_rita_buf_push(&vbuf_points, (HglRitaVertex){.pos.x = 600, .pos.y = 100, .color = HGL_RITA_WHITE});

    /* vertex buffer: Line Strip */
    HglRitaVertexBuffer vbuf_line_strip = {0};
    hgl_rita_buf_push(&vbuf_line_strip, (HglRitaVertex){.pos.x = 700, .pos.y =  50, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf_line_strip, (HglRitaVertex){.pos.x = 720, .pos.y =  60, .color = HGL_RITA_GREEN});
    hgl_rita_buf_push(&vbuf_line_strip, (HglRitaVertex){.pos.x = 740, .pos.y =  80, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf_line_strip, (HglRitaVertex){.pos.x = 760, .pos.y = 120, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf_line_strip, (HglRitaVertex){.pos.x = 780, .pos.y = 200, .color = HGL_RITA_GREEN});
    hgl_rita_buf_push(&vbuf_line_strip, (HglRitaVertex){.pos.x = 800, .pos.y = 360, .color = HGL_RITA_BLUE});

    /* vertex buffer: Triangle strip */
    HglRitaVertexBuffer vbuf_tri_strip = {0};
    hgl_rita_buf_push(&vbuf_tri_strip, (HglRitaVertex){.pos.x = 100, .pos.y = 500, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf_tri_strip, (HglRitaVertex){.pos.x = 100, .pos.y = 600, .color = HGL_RITA_RED});
    hgl_rita_buf_push(&vbuf_tri_strip, (HglRitaVertex){.pos.x = 200, .pos.y = 500, .color = HGL_RITA_GREEN});
    hgl_rita_buf_push(&vbuf_tri_strip, (HglRitaVertex){.pos.x = 200, .pos.y = 600, .color = HGL_RITA_GREEN});
    hgl_rita_buf_push(&vbuf_tri_strip, (HglRitaVertex){.pos.x = 300, .pos.y = 500, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf_tri_strip, (HglRitaVertex){.pos.x = 300, .pos.y = 600, .color = HGL_RITA_BLUE});
    hgl_rita_buf_push(&vbuf_tri_strip, (HglRitaVertex){.pos.x = 400, .pos.y = 500, .color = HGL_RITA_MAGENTA});
    hgl_rita_buf_push(&vbuf_tri_strip, (HglRitaVertex){.pos.x = 400, .pos.y = 600, .color = HGL_RITA_MAGENTA});

    /* vertex buffer: Triangle fan */
    HglRitaVertexBuffer vbuf_tri_fan = {0};
    hgl_rita_buf_push(&vbuf_tri_fan, (HglRitaVertex){.pos.x =  500, .pos.y = 900, .color = HGL_RITA_RED});
    int n = 16;
    HglRitaColor start_color = HGL_RITA_GREEN;
    HglRitaColor end_color = HGL_RITA_BLUE;
    for (int i = 0; i <= n; i++) {
        float t = (float)i / (float)n;
        float angle = -t * 3.1415926535f / 2.0;
        hgl_rita_buf_push(&vbuf_tri_fan, (HglRitaVertex){.pos.x = 500 + 400*cosf(angle), 
                                                         .pos.y = 900 + 400*sinf(angle), 
                                                         .color = hgl_rita_color_lerp(start_color, end_color, t)});
    }

    /* Raylib stuff: IGNORE */
    InitWindow(DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, "HglRita: Primitives");
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

        hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf_tris);
        hgl_rita_draw(HGL_RITA_TRIANGLES);

        hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf_lines);
        hgl_rita_draw(HGL_RITA_LINES);

        hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf_points);
        hgl_rita_draw(HGL_RITA_POINTS);

        hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf_line_strip);
        hgl_rita_draw(HGL_RITA_LINE_STRIP);

        hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf_tri_strip);
        hgl_rita_draw(HGL_RITA_TRIANGLE_STRIP);

        hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf_tri_fan);
        hgl_rita_draw(HGL_RITA_TRIANGLE_FAN);

        hgl_rita_finish();
       
        //sleep(1);
        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
            DrawFPS(10,10);
        EndDrawing();
    }

    CloseWindow();
}
