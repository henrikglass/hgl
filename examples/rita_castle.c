#define _DEFAULT_SOURCE

#define HGL_RITA_VERTEX_SPEC_PRESET_3D_POS_UV
#define HGL_RITA_RENDERER_PRESET_256X64X2048_PARALLEL_VERTEX_PROCESSING
#define HGL_RITA_STRIP_PREFIX
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
    rita_init();
    RitaTexture fb_color = rita_texture_make(WIDTH, HEIGHT, RITA_RGBA8);
    RitaTexture fb_depth = rita_texture_make(WIDTH, HEIGHT, RITA_R32);
    rita_bind_texture(RITA_TEX_FRAME_BUFFER, &fb_color);
    rita_bind_texture(RITA_TEX_DEPTH_BUFFER, &fb_depth);
    rita_use_viewport(WIDTH, HEIGHT);

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

    rita_use_perspective_proj(3.14f/4, (float)(WIDTH)/(float)(HEIGHT), 2.0f, 1000.0f);
    rita_enable(RITA_BACKFACE_CULLING |
                RITA_DEPTH_TESTING);
    MyModel model = load_model_from_obj("assets/castle.obj");
    model.diffuse = load_texture("assets/castle4k.png");
    model.winding_order = RITA_CCW;
    model.tform = mat4_scale(model.tform, vec3(1.8f, 1.8f, 1.8f));
    model.tform = mat4_translate(model.tform, vec3(0.0, -30.0, 0));
    rita_use_vertex_buffer_mode(RITA_INDEXED);
    rita_bind_buffer(RITA_VERTEX_BUFFER, &model.vbuf);
    rita_bind_buffer(RITA_INDEX_BUFFER, &model.ibuf);
    rita_use_model_matrix(model.tform);
    rita_use_frontface_winding_order(model.winding_order);
    rita_bind_texture(RITA_TEX_DIFFUSE, &model.diffuse);
    rita_use_texture_filter(RITA_NEAREST);
    
    RitaTexture skybox;
    skybox = load_texture("assets/skybox_cubemap.png");

    //SetTargetFPS(60);
    int frame_count = 0;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* draw */
        rita_clear(RITA_DEPTH);
        rita_draw(RITA_TRIANGLES);
        rita_blit(0, 0, WIDTH, HEIGHT, &skybox, 
                  RITA_REPLACE, 
                  RITA_DEPTH_INF, 
                  RITA_VIEW_DIR_CUBEMAP,
                  NULL);
        rita_finish();


        /* update */
        float d = 80;
        //Mat4 view = mat4_look_at(vec3(d*sinf(0.01*frame_count), 20, d*cosf(0.01*frame_count)), 
        //                         vec3(0, 0, 0), vec3(0,1,0));
        rita_use_camera_view(vec3(d*sinf(0.01*frame_count), 20, d*cosf(0.01*frame_count)), 
                             vec3(0, 0, 0), vec3(0,1,0));

        /* raylib stuff */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
            DrawFPS(10,10);
        EndDrawing();

        frame_count++;
    }


    CloseWindow();
    rita_final();

    return 0;
}

