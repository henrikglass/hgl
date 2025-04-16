#define _DEFAULT_SOURCE

#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "rita_helpers.h"

#include "raylib.h"
#include "stb_image.h"

static HglRitaTexture skybox;
static Vec3 camera_pos;

HglRitaColor my_shader(const struct HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    Vec3 v_in;
    Vec3 v_out;
    float fresnel;
    HglRitaColor base = HGL_RITA_DARK_RED;
    HglRitaColor specular;

    v_in = vec3_sub(in->world_pos, camera_pos);
    v_in = vec3_normalize(v_in);
    v_out = vec3_reflect(v_in, vec3_normalize(in->world_normal));
    fresnel = powf(1.0f - clamp(0,1,-vec3_dot(in->world_normal, v_in)), 1.2f);
    specular = hgl_rita_sample_cubemap(&skybox, v_out);
    specular = hgl_rita_color_mul_scalar(specular, fresnel);

    return hgl_rita_color_add(base, specular);
}

int main()
{
    const int WIDTH = 800;
    const int HEIGHT = 600;

    /* Initialize hgl_rita */
    hgl_rita_init();
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Create a framebuffer texture & depth buffer texture and bind them */
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    HglRitaTexture fb_depth = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_R32);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_bind_texture(HGL_RITA_TEX_DEPTH_BUFFER, &fb_depth);

    /* Setup the camera (the view matrix is created in the draw loop) */
    hgl_rita_use_perspective_proj(3.1415f/3.0f, (float)WIDTH/(float)HEIGHT, 2.0f, 1000.0f);

    /* Specify the viewport (maps NDC:s to x \in [0,800], y \in [0,600]) */
    hgl_rita_use_viewport(WIDTH, HEIGHT);

    /* Enable all the fancy stuff */
    hgl_rita_enable(HGL_RITA_BACKFACE_CULLING |
                    HGL_RITA_DEPTH_TESTING |
                    HGL_RITA_DEPTH_BUFFER_WRITING |
                    HGL_RITA_Z_CLIPPING);

    /* Load a teapot model */
    MyModel teapot = load_model_from_obj("assets/teapot.obj");

    /* Setup hgl_rita to render our model */
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &teapot.vbuf);
    hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &teapot.ibuf);
    hgl_rita_use_frontface_winding_order(teapot.winding_order);
    hgl_rita_use_model_matrix(teapot.tform);
    
    /* Load a texture for our skybox */
    skybox = load_texture("assets/skybox_cubemap.png");

    /* Setup hgl_rita to use our custom shader */
    hgl_rita_bind_frag_shader(my_shader);

    /* Raylib stuff: IGNORE */
    InitWindow(2*WIDTH, 2*HEIGHT, "HglRita: Skybox");
    Image color_image = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    int frame = 0;
    SetTargetFPS(60);
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH); // TODO don't need to clear color
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        hgl_rita_blit(0, 0, WIDTH, HEIGHT, &skybox, 
                      HGL_RITA_REPLACE_SKIP_ALPHA, 
                      HGL_RITA_DEPTH_INF, 
                      HGL_RITA_VIEW_DIR_CUBEMAP,
                      NULL);
        hgl_rita_finish();

        /* animate the camera */
        float d = 35;
        float h = 20;
        camera_pos = vec3_make(d*cosf(frame*0.01f), 
                               h*sinf(frame*0.05f), 
                               d*sinf(frame*0.01f));
        hgl_rita_use_camera_view(
            camera_pos, 
            vec3_make(0, 0, 0), 
            vec3_make(0, 1, 0)
        );

        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, 2, WHITE);
            DrawFPS(10,10);
        EndDrawing();
        frame++;
    }

    CloseWindow();
}
