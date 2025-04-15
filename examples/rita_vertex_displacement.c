#define _DEFAULT_SOURCE

#define HGL_RITA_IMPLEMENTATION
#define HGL_RITA_SHADERS_IMPLEMENTATION
#include "hgl_rita.h"
#include "hgl_rita_shaders.h"

#include "rita_helpers.h"

#include "raylib.h"
#include "stb_image.h"


#define WIDTH         800
#define HEIGHT        600
#define DISPLAY_SCALE   2


static int frame_count = 0;

static inline HglRitaVertex my_vertex_displacement_shader(const HglRitaContext *ctx, const HglRitaVertex *in)
{
    HglRitaVertex out;
    Vec4 v;

    /* get local space vertex pos and extend to 4D */
    v = in->pos;
    v.w = 1.0f;

    v.y += (v.x+10)*0.15f*sinf(frame_count*0.1f + v.x * 0.3f + v.z * 0.2f);

    /* local space --> world space -> view space */
    v = mat4_mul_vec4(ctx->tform.mvp, v);

    out.pos     = v;
    out.normal  = in->normal;
    out.tangent = in->tangent;
    out.uv      = in->uv;
    out.color   = in->color;

    return out;
}

int main()
{
    /* Initialize hgl_rita */
    hgl_rita_init();

    /* Create a framebuffer texture & depth buffer texture and bind them */
    HglRitaTexture fb_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8);
    HglRitaTexture fb_depth = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_R32);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color);
    hgl_rita_bind_texture(HGL_RITA_TEX_DEPTH_BUFFER, &fb_depth);
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Setup the camera (the view matrix is created in the draw loop) */
    //Mat4 view = mat4_look_at(vec3_make(30,15,0), vec3_make(0, 0, 0), vec3_make(0, 1, 0));
    Mat4 view = mat4_look_at(vec3_make(0,15,30), vec3_make(0, 0, 0), vec3_make(0, 1, 0));
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
    MyModel teapot = load_model_from_obj("assets/plane16x16.obj");

    /* Setup hgl_rita to render our model */
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &teapot.vbuf);
    hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &teapot.ibuf);
    hgl_rita_use_frontface_winding_order(teapot.winding_order);
    hgl_rita_use_model_matrix(teapot.tform);
   
    /* bind our vertex displacement shader */
    hgl_rita_bind_vert_shader(my_vertex_displacement_shader);
    hgl_rita_bind_frag_shader(HGL_RITA_LAMBERT_DIFFUSE);

    HglRitaTexture tex = load_texture("assets/rainbow_gradient.png");
    hgl_rita_bind_texture(HGL_RITA_TEX_DIFFUSE, &tex);

    /* Raylib stuff: IGNORE */
    InitWindow(DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, "HglRita: Vertex displacement");
    Image color_image = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    SetTargetFPS(60);
    bool pause_animation = false;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        hgl_rita_finish();

        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
            DrawFPS(10,10);
        EndDrawing();

        if (IsKeyPressed(KEY_P)) {
            pause_animation = !pause_animation;
        }
        if (!pause_animation) {
            frame_count++;
        }
    }

    CloseWindow();
}
