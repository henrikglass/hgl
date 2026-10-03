#define _DEFAULT_SOURCE

//#define HGL_RITA_PARALLEL_VERTEX_PROCESSING
//#define HGL_RITA_RENDERER_PRESET_SINGLE_THREAD

//#define HGL_RITA_VERTEX_SPEC_PRESET_2D_POS_COLOR
//#define HGL_RITA_RENDERER_PRESET_SINGLE_THREAD
#define HGL_RITA_IMPLEMENTATION
//#include "hgl_rita.h"
#include "hgl_rita.h"

#include "raylib.h"
#include "stb_image.h"

#define WIDTH          512
#define HEIGHT         512

int main()
{
    printf("%zu\n", sizeof(HglRitaOp));
    printf("%zu\n", sizeof(HglRitaBlitInfo));
    printf("%zu\n", sizeof(HglRitaTriangle));

    /* Create a framebuffer texture and bind it (depth buffer is optional) */
    hgl_rita_init();
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);
    hgl_rita_use_viewport(WIDTH, HEIGHT);
    Mat4 view = mat4_ortho(0, WIDTH, HEIGHT, 0, -10, 10);
    hgl_rita_use_view_matrix(view);
    hgl_rita_disable(HGL_RITA_DEPTH_BUFFER_WRITING);
    hgl_rita_disable(HGL_RITA_DEPTH_TESTING);
    hgl_rita_enable(HGL_RITA_BACKFACE_CULLING);
    hgl_rita_enable(HGL_RITA_ORDER_DEPENDENT_ALPHA_BLEND);
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_ARRAY);

    /* vertex buffer: Smooth Lines */
    HglRitaVertexBuffer vbuf_lines = {0};
    int N = 64;

    //float a = (float)HGLM_PI*2.0f*((float)5/(float)N);
    //hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 800, .pos.y =  256, .color = HGL_RITA_MORTEL_WHITE});
    //hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 200*cosf(a) + 800, .pos.y = 200*sinf(a) + 256, .color = HGL_RITA_MORTEL_WHITE});
    //vec2_print(vbuf_lines.arr[0].pos);
    //vec2_print(vbuf_lines.arr[1].pos);

    /* Raylib stuff: IGNORE */
    InitWindow(2*WIDTH, 2*HEIGHT, "HglRita: lines");
    Image color_image = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    float d = 0.0f;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR);
        hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf_lines);

        hgl_rita_buf_clear(&vbuf_lines);
        for (int i = 0; i < N; i++) {
            float a = (float)HGLM_PI*2.0f*(d+(float)i/(float)N);
            hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 256, .pos.y =  256, .color = HGL_RITA_MORTEL_MAGENTA});
            hgl_rita_buf_push(&vbuf_lines, (HglRitaVertex){.pos.x = 200.0f*cosf(a) + 256, .pos.y = 200.0f*sinf(a) + 256, .color = HGL_RITA_MORTEL_CYAN});
        }
        hgl_rita_draw(HGL_RITA_LINES_SMOOTH);
        hgl_rita_finish();
       
        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, 2.0f, WHITE);
            DrawFPS(10,10);
        EndDrawing();

        d += 0.00001f;
    }

    CloseWindow();
}
