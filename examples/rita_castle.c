#define _DEFAULT_SOURCE

#define HGL_RITA_SIMPLE
#define HGL_RITA_PRESET_256X64X2048_PARALLEL_VERTEX_PROCESSING
#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"
#include "hgl_rita_shaders.h"
#include "rita_helpers.h"

#include "raylib.h"

#include "stb_image.h"

#include <stdio.h>

#define WIDTH (1920)
#define HEIGHT (1080)
#define DISPLAY_SCALE 1

int main()
{

    /* rita stuff */
    hgl_rita_init();
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    HglRitaTexture fb_depth = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_R32);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_bind_texture(HGL_RITA_TEX_DEPTH_BUFFER, &fb_depth);
    hgl_rita_use_viewport(WIDTH, HEIGHT);

    /* raylib stuff */
    InitWindow(DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, "HglRita test");
    Image color_image = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    Mat4 view   = mat4_look_at(vec3_make(0,0,100), vec3_make(0, 0, 0), vec3_make(0,1,0));
    Mat4 proj   = mat4_make_perspective(3.14f/4, (float)(WIDTH)/(float)(HEIGHT), 2.0f, 1000.0f);
    hgl_rita_use_view_matrix(view);
    hgl_rita_use_proj_matrix(proj);
    hgl_rita_enable(HGL_RITA_Z_CLIPPING |
                    HGL_RITA_BACKFACE_CULLING |
                    HGL_RITA_DEPTH_TESTING |
                    HGL_RITA_ORDER_DEPENDENT_ALPHA_BLEND);
    MyModel model = load_model_from_obj("assets/castle.obj");
    model.diffuse = load_texture("assets/castle4k.png");
    model.winding_order = HGL_RITA_CCW;
    model.tform = mat4_scale(model.tform, vec3_make(1.8f, 1.8f, 1.8f));
    model.tform = mat4_translate(model.tform, vec3_make(0.0, -30.0, 0));
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &model.vbuf);
    hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &model.ibuf);
    hgl_rita_use_model_matrix(model.tform);
    hgl_rita_use_frontface_winding_order(model.winding_order);
    hgl_rita_bind_texture(HGL_RITA_TEX_DIFFUSE, &model.diffuse);
    
    HglRitaTexture skybox;
    skybox = load_texture("assets/skybox_cubemap.png");
    hgl_rita_texture_flip_vertically(&skybox);

    SetTargetFPS(60);
    int frame_count = 0;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* draw */
        hgl_rita_clear(HGL_RITA_DEPTH);
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        hgl_rita_use_texture_filter(HGL_RITA_NEAREST);
        hgl_rita_blit(0, 0, WIDTH, HEIGHT, skybox, 
                      HGL_RITA_REPLACE, 
                      HGL_RITA_DEPTH_INF, 
                      HGL_RITA_VIEW_DIR_CUBEMAP,
                      NULL);
        hgl_rita_finish();


        /* update */
        float d = 80;
        Mat4 view = mat4_look_at(vec3_make(d*sinf(0.01*frame_count), 20, d*cosf(0.01*frame_count)), 
                                 vec3_make(0, 0, 0), vec3_make(0,1,0));
        hgl_rita_use_view_matrix(view);

        /* raylib stuff */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
            DrawFPS(10,10);
        EndDrawing();

        frame_count++;
    }


    CloseWindow();
    hgl_rita_final();

    return 0;
}

