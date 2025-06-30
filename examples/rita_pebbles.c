#define _DEFAULT_SOURCE

#define HGL_RITA_PRESET_256X64X256_SERIAL_VERTEX_PROCESSING
#include "hgl_rita.h"
#include "rita_helpers.h"

#include "raylib.h"
#include "stb_image.h"

#define WIDTH          (1920)
#define HEIGHT         (1080)
#define DISPLAY_SCALE    1.0

static int frame_count = 0;
static Vec3 camera_pos;

static inline HglRitaVertex my_vertex_shader(const HglRitaContext *ctx, const HglRitaVertex *in)
{
    HglRitaVertex out;
    Vec4 v;

    /* get local space vertex pos and extend to 4D */
    v = in->pos;
    v.w = 1.0f;

    float h = hgl_rita_sample_unit_uv(HGL_RITA_TEX_DISPLACEMENT, in->uv).r / 255.0f;
    v.y += 1.4f*h;

    /* local space --> world space -> view space */
    v = mat4_mul_vec4(ctx->tform.mvp, v);

    out.pos     = v;
    out.normal  = mat3_mul_vec3(ctx->tform.normals, in->normal);
    out.tangent = mat3_mul_vec3(ctx->tform.normals, in->tangent);
    out.uv      = in->uv;
    out.color   = in->color;

    return out;
}

static inline HglRitaColor my_fragment_shader(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    Vec3 L  = vec3_normalize(vec3_make(1,1, sinf(frame_count*0.09f))); // light vector
    Vec3 IV = vec3_normalize(vec3_sub(in->world_pos, camera_pos)); // inverse view vector
    Vec3 N  = in->world_normal;
    Vec3 T  = in->world_tangent;
    Vec3 B  = vec3_cross(N, T);
    Mat3 TBN = mat3_make(T, B, N); 

    HglRitaColor n0c = hgl_rita_sample_unit_uv(HGL_RITA_TEX_NORMAL, in->uv);
    Vec3 n0;
    n0.x = (n0c.r/255.0f) * 2.0f - 1.0f;
    n0.y = (n0c.g/255.0f) * 2.0f - 1.0f;
    n0.z = (n0c.b/255.0f) * 2.0f - 1.0f;
    N = vec3_normalize(mat3_mul_vec3(TBN, n0));

    float gloss = 15.0f;
    HglRitaColor specular_color = HGL_RITA_WHITE;

    HglRitaColor color = HGL_RITA_BLACK;
    HglRitaColor diffuse_color = in->color;
    diffuse_color = hgl_rita_sample_unit_uv(HGL_RITA_TEX_DIFFUSE, in->uv);
    float diffuse_light = 0.1f + fmaxf(0, vec3_dot(N, L)); // Lambertian
    float specular_light = powf(fmaxf(0, vec3_dot(vec3_reflect(IV, N), L)), gloss); // Phong

    diffuse_color.r *= diffuse_light;
    diffuse_color.g *= diffuse_light;
    diffuse_color.b *= diffuse_light;

    specular_color.r *= specular_light * 0.55f;
    specular_color.g *= specular_light * 0.55f;
    specular_color.b *= specular_light * 0.55f;

    color = hgl_rita_color_add(diffuse_color, specular_color);
    return color;

}

#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"
#include "hgl_rita_shaders.h"

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

    /* Setup the camera (the view matrix is created in the draw loop) */
    camera_pos = vec3_make(0, 0, 0);
    Mat4 proj = mat4_make_perspective(3.1415f/4.0f, (float)WIDTH/(float)HEIGHT, 2.0f, 1000.0f);
    hgl_rita_use_proj_matrix(proj);

    /* Specify the viewport (maps NDC:s to x \in [0,800], y \in [0,600]) */
    hgl_rita_use_viewport(WIDTH, HEIGHT);

    /* Enable all the fancy stuff */
    hgl_rita_enable(HGL_RITA_BACKFACE_CULLING |
                    HGL_RITA_DEPTH_TESTING |
                    HGL_RITA_DEPTH_BUFFER_WRITING |
                    HGL_RITA_Z_CLIPPING);

    /* Load a model */
    MyModel model = load_model_from_obj("assets/plane64x64.obj");

    /* Setup hgl_rita to render our model */
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &model.vbuf);
    hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &model.ibuf);
    hgl_rita_use_frontface_winding_order(model.winding_order);
    hgl_rita_use_model_matrix(model.tform);
   
    hgl_rita_bind_vert_shader(my_vertex_shader);
    hgl_rita_bind_frag_shader(my_fragment_shader);
    
    /* load and setup our textures */
    HglRitaTexture diffuse_map = load_texture("assets/pebbles/diffuse.png");
    HglRitaTexture displacement_map = load_texture("assets/pebbles/displacement.png");
    HglRitaTexture normal_map = load_texture("assets/pebbles/normal.png");
    hgl_rita_bind_texture(HGL_RITA_TEX_DIFFUSE, &diffuse_map);
    hgl_rita_bind_texture(HGL_RITA_TEX_DISPLACEMENT, &displacement_map);
    hgl_rita_bind_texture(HGL_RITA_TEX_NORMAL, &normal_map);
    hgl_rita_use_texture_filter(HGL_RITA_NEAREST);
    hgl_rita_use_texture_wrapping(HGL_RITA_NO_WRAPPING);

    /* Raylib stuff: IGNORE */
    InitWindow(DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, "HglRita: Vertex displacement maps");
    Image color_image = (Image) {
        .data    = fb_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    //SetTargetFPS(60);
    bool pause_animation = false;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        /* Draw! */
        hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
        hgl_rita_draw(HGL_RITA_TRIANGLES);
        //hgl_rita_draw(HGL_RITA_TRIANGLES_DISPATCH);
        hgl_rita_finish();

        float t1 = clamp(0, 0.99, 0.5f + 1.4f*sinf(frame_count*0.03f));
        float t2 = clamp(0, 0.99, 0.5f + 1.4f*sinf(frame_count*0.03f + 3.1415f));
        camera_pos = vec3_lerp(vec3_make(0,20,0),vec3_make(0,20,20), t1);
        Mat4 view = mat4_look_at(camera_pos, 
                                 vec3_make(0,0,0), 
                                 vec3_lerp(vec3_make(0,0,-1),vec3_make(0,1,0), t2));
        if (!pause_animation) {
            hgl_rita_use_view_matrix(view);
        }

        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, color_image.data);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
            DrawFPS(10,10);
        EndDrawing();

        if (IsKeyPressed(KEY_P)) {
            pause_animation = !pause_animation;
        }
        frame_count++;
    }

    CloseWindow();
}
