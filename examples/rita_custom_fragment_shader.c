#define _DEFAULT_SOURCE

#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "rita_helpers.h"

#include "raylib.h"
#include "stb_image.h"

struct {
    Vec3 pos;
    HglRitaColor color;
    float brightness;
} lights[] = {
    {.pos = (Vec3){.x = -25, .y = 15, .z = 15}, .color = HGL_RITA_RED, .brightness = 200.0f},
    {.pos = (Vec3){.x =   0, .y = 15, .z = 15}, .color = HGL_RITA_GREEN, .brightness = 200.0f},
    {.pos = (Vec3){.x =  25, .y = 15, .z = 15}, .color = HGL_RITA_BLUE, .brightness = 200.0f},
};

HglRitaColor my_shader(const struct HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    (void) in;

    HglRitaColor color = HGL_RITA_BLACK;

    for (size_t i = 0; i < sizeof(lights)/sizeof(lights[0]); i++) {
        Vec3 f_to_l = vec3_sub(lights[i].pos, in->world_pos);
        float d = vec3_len(f_to_l);
        f_to_l = vec3_normalize(f_to_l);
        float intensity = clamp(0, 1, vec3_dot(vec3_normalize(in->world_normal), f_to_l));
        intensity *= lights[i].brightness / (d*d);
        color = hgl_rita_color_add(color, hgl_rita_color_mul_scalar(lights[i].color, intensity));
    } 

    return color;
}

int main()
{
    const int WIDTH = 800;
    const int HEIGHT = 600;

    /* Initialize hgl_rita */
    hgl_rita_init();

    /* Create a framebuffer texture & depth buffer texture and bind them */
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    HglRitaTexture fb_depth = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_R32);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_bind_texture(HGL_RITA_TEX_DEPTH_BUFFER, &fb_depth);
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Setup the camera */
    Mat4 view = mat4_look_at(vec3_make(0, 10, 100), vec3_make(0,  0,   0), vec3_make(0,  1,   0));
    Mat4 proj = mat4_make_perspective(3.1415f/4.0f, (float)WIDTH/(float)HEIGHT, 2.0f, 1000.0f);
    hgl_rita_use_view_matrix(view);
    hgl_rita_use_proj_matrix(proj);

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

    /* Setup hgl_rita to use our custom fragment shader */
    hgl_rita_bind_frag_shader(my_shader);

    /* Raylib stuff: IGNORE */
    InitWindow(WIDTH, HEIGHT, "HglRita: Custom fragment shader!");
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
        hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        hgl_rita_finish();

        /* move the teapot a tiny bit per frame. */
        teapot.tform = mat4_make_rotation(0.01*frame, vec3_make(0,1,0));
        teapot.tform = mat4_translate(teapot.tform, vec3_make(30*sinf(frame*0.01f),0.0,0.0));
        hgl_rita_use_model_matrix(teapot.tform);

        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTexture(color_tex, 0, 0, WHITE);
            DrawFPS(10,10);
        EndDrawing();
        frame++;
    }

    CloseWindow();
}
