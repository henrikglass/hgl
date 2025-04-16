#define _DEFAULT_SOURCE

#define HGL_RITA_PRESET_256X64X256_PARALLEL_VERTEX_PROCESSING
#define HGL_RITA_IMPLEMENTATION
#define HGL_RITA_SHADERS_IMPLEMENTATION
#include "hgl_rita.h"
#include "hgl_rita_shaders.h"
#include "rita_helpers.h"

#define HGL_PROFILE_IMPLEMENTATION
#include "hgl_profile.h"

#include "raylib.h"
#include "stb_image.h"

static HglRitaTexture *render_texture = NULL;
static HglRitaTexture *frame_buffer = NULL;

static inline HglRitaColor sobel_sharpen(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;
    /*
     * Applies the sobel kernel:
     *
     *      |  0, -1,  0 |
     *      | -1,  5, -1 |
     *      |  0, -1,  0 |
     */
    HglRitaColor texel;
    int r = 0;
    int g = 0;
    int b = 0;
    texel = hgl_rita_sample(render_texture, in->x, in->y - 1);
    r += -1.0f * texel.r;
    g += -1.0f * texel.g;
    b += -1.0f * texel.b;
    texel = hgl_rita_sample(render_texture, in->x - 1, in->y);
    r += -1.0f * texel.r;
    g += -1.0f * texel.g;
    b += -1.0f * texel.b;
    texel = hgl_rita_sample(render_texture, in->x, in->y);
    r += 5.0f * texel.r;
    g += 5.0f * texel.g;
    b += 5.0f * texel.b;
    texel = hgl_rita_sample(render_texture, in->x + 1, in->y);
    r += -1.0f * texel.r;
    g += -1.0f * texel.g;
    b += -1.0f * texel.b;
    texel = hgl_rita_sample(render_texture, in->x, in->y + 1);
    r += -1.0f * texel.r;
    g += -1.0f * texel.g;
    b += -1.0f * texel.b;
    return (HglRitaColor) {
        .r = clamp(0, 255, r),
        .g = clamp(0, 255, g),
        .b = clamp(0, 255, b),
        .a = 255
    };
}

static struct {
    const char *name;
    HglRitaVertShaderFunc func;
} vertex_shaders[] = {
    {.name = "NULL",                .func = NULL},
    {.name = "VERTEX SNAPPING",     .func = HGL_RITA_VERTEX_SNAP_SHADER},
    {.name = "VERTEX LIGHT",        .func = HGL_RITA_VERTEX_DIRECTIONAL_LIGHT_SHADER},
};

static struct {
    const char *name;
    HglRitaFragShaderFunc func;
} fragment_shaders[] = {
    {.name = "NULL",                .func = NULL},
    {.name = "UV",                  .func = HGL_RITA_UV_SHADER},
    {.name = "NORMALS",             .func = HGL_RITA_NORMAL_SHADER},
    {.name = "DEPTH",               .func = HGL_RITA_DEPTH_SHADER},
    {.name = "LAMBERT DIFFUSE",     .func = HGL_RITA_LAMBERT_DIFFUSE},
    {.name = "GOOCH",               .func = HGL_RITA_GOOCH},
    {.name = "PHONG",               .func = HGL_RITA_PHONG},
    {.name = "BLINN-PHONG",         .func = HGL_RITA_BLINN_PHONG},
};

static struct {
    const char *name;
    HglRitaFragShaderFunc func;
} filter_shaders[] = {
    {.name = "NULL",                .func = NULL},
    {.name = "MONOCHROME",          .func = HGL_RITA_MONOCHROME},
    {.name = "FOG",                 .func = HGL_RITA_FOG},
    {.name = "DEPTH_BASED_BORDERS", .func = HGL_RITA_DEPTH_BASED_BORDERS},
    {.name = "SOBEL_SHARPEN",       .func = sobel_sharpen},
    {.name = "DITHER_4X4_1BPP",     .func = HGL_RITA_DITHER_4X4_1BPP},
    {.name = "DITHER_4X4_2BPP",     .func = HGL_RITA_DITHER_4X4_2BPP},
    {.name = "DITHER_4X4_3BPP",     .func = HGL_RITA_DITHER_4X4_3BPP},
};

#define WIDTH          (800)
#define HEIGHT         (600)
#define DISPLAY_SCALE   2.0

int main()
{

    /* Initialize hgl_rita */
    hgl_rita_init();
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);

    /* Create a framebuffer texture & depth buffer texture and bind them */
    HglRitaTexture fb_color[2] = {
        hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8),
        hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8),
    };
    HglRitaTexture fb_depth = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_R32);
    hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &fb_color[0]);
    hgl_rita_bind_texture(HGL_RITA_TEX_DEPTH_BUFFER, &fb_depth);

    /* Setup the camera */
    float camera_distance = 100.0f;
    float camera_angle = 0;
    float camera_height = 20;
    Vec3 camera_pos = vec3_make(100*sinf(camera_angle), camera_height, 100*cosf(camera_angle));
    hgl_rita_use_perspective_proj(3.1415f/3.0f,               // FOV (60 degrees)
                                  (float)WIDTH/(float)HEIGHT, // aspect ratio
                                  2.0f,                       // near clipping plane
                                  1000.0f);                   // far clipping plane

    /* Specify the viewport (maps NDC:s to x \in [0,800], y \in [0,600]) */
    hgl_rita_use_viewport(WIDTH, HEIGHT);

    /* Enable all the fancy stuff */
    hgl_rita_enable(HGL_RITA_BACKFACE_CULLING | 
                    HGL_RITA_DEPTH_TESTING |
                    HGL_RITA_DEPTH_BUFFER_WRITING | 
                    HGL_RITA_Z_CLIPPING);

    /* Load models */
    MyModel models[4];
    memset(models, 0, sizeof(models));
    models[0] = load_model_from_obj("assets/cube.obj");
    models[0].diffuse = load_texture("assets/box64x64.png");
    models[0].tform = mat4_scale(models[0].tform, vec3_make(10, 10, 10));
    models[1] = load_model_from_obj("assets/skull.obj");
    models[1].diffuse = load_texture("assets/skull.png");
    models[1].tform = mat4_scale(models[1].tform, vec3_make(1, 1, 1));
    models[1].tform = mat4_rotate(models[1].tform, -3.1415/2.0f, vec3_make(1, 0, 0));
    models[1].tform = mat4_translate(models[1].tform, vec3_make(0, -10, 0));
    models[2] = load_model_from_obj("assets/hcandersen.obj");
    models[2].diffuse = load_texture("assets/hcandersen_albedo.png");
    models[2].tform = mat4_scale(models[2].tform, vec3_make(1, 1, 1));
    models[2].tform = mat4_translate(models[2].tform, vec3_make(0, -20, 0));
    models[3] = load_model_from_obj("assets/cavetroll.obj");
    models[3].diffuse = load_texture("assets/cavetroll.png");
    models[3].tform = mat4_scale(models[2].tform, vec3_make(8, 8, 8));

    /* Setup hgl_rita to render our models */
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);

    /* Load skybox texture */
    HglRitaTexture skybox;
    skybox = load_texture("assets/skybox_cubemap.png");

    /* Raylib stuff: IGNORE */
    InitWindow(DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, "HglRita: Hello Cube!");
    Image color_image = (Image) {
        .data    = fb_color[0].data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    hgl_rita_bind_vert_shader(HGL_RITA_VERTEX_DIRECTIONAL_LIGHT_SHADER);

    SetTargetFPS(60);
    int frag_shader_in_use     = 0;
    int vert_shader_in_use     = 0;
    int filter_shader_in_use   = 0;
    int model_in_use           = 0;
    int background_in_use      = 0;
    bool depth_testing         = true;
    bool backface_culling      = true;
    bool frontface_winding_ccw = true;
    bool use_perspective_project   = true;
    bool wireframes            = false;
    int frame_count = 0;
    render_texture = &fb_color[0];
    frame_buffer = &fb_color[1];
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q)) {
        /* Only sobel_sharpen needs drawing to a render texture */
        if (filter_shaders[filter_shader_in_use].func == sobel_sharpen) {
            render_texture = &fb_color[frame_count % 2];
            frame_buffer = &fb_color[(frame_count + 1) % 2];
            hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, render_texture);
        } else {
            hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, frame_buffer);
        }

        /* Bind our models vertex buffer, index buffer, model matrix, and diffuse texture */
        hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &models[model_in_use].vbuf);
        hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &models[model_in_use].ibuf);
        hgl_rita_use_model_matrix(models[model_in_use].tform);
        hgl_rita_bind_texture(HGL_RITA_TEX_DIFFUSE, &models[model_in_use].diffuse);

        switch (background_in_use) {
            case 0: {
                hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);
                hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
            } break;

            case 1: {
                hgl_rita_use_clear_color(HGL_RITA_MORTEL_WHITE);
                hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
            } break;

            case 2: {
                hgl_rita_clear(HGL_RITA_DEPTH);
            } break;
        }

        /* Draw model */
        hgl_rita_draw(HGL_RITA_TRIANGLES);

        /* Blit */
        /* Maybe draw skybox */
        if (background_in_use == 2) {
            hgl_rita_blit(0, 0, WIDTH, HEIGHT, &skybox, 
                          HGL_RITA_REPLACE, 
                          HGL_RITA_DEPTH_INF, 
                          HGL_RITA_VIEW_DIR_CUBEMAP,
                          NULL);
        }

        /* Maybe apply post-processing shader */
        hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, frame_buffer); // necessary only for sobel_sharpen
        if (filter_shaders[filter_shader_in_use].func != NULL) {
            hgl_rita_blit(0, 0, WIDTH, HEIGHT, frame_buffer, 
                          HGL_RITA_REPLACE, 
                          HGL_RITA_EVERYWHERE, 
                          HGL_RITA_SHADER,
                          filter_shaders[filter_shader_in_use].func);
        }

        /* Draw text */
        hgl_rita_draw_text(10,  16, 1.0f, HGL_RITA_MORTEL_GREEN, "FPS: %d", GetFPS());
        hgl_rita_draw_text(10,  16 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press A to cycle between fragment shaders: %s", fragment_shaders[frag_shader_in_use].name);
        hgl_rita_draw_text(10,  32 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press S to toggle backface culling on/off");
        hgl_rita_draw_text(10,  48 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press D to toggle depth testing on/off");
        hgl_rita_draw_text(10,  64 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press F to toggle frontface winding order CW/CCW");
        hgl_rita_draw_text(10,  80 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press UP/DOWN/LEFT/RIGHT/LCTRL/LSHIFT to move camera");
        hgl_rita_draw_text(10,  96 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press G to cycle between filter shaders: %s", filter_shaders[filter_shader_in_use].name);
        hgl_rita_draw_text(10, 112 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press H to cycle between vertex shaders: %s", vertex_shaders[vert_shader_in_use].name);
        hgl_rita_draw_text(10, 128 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press M to cycle between models");
        hgl_rita_draw_text(10, 144 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press B to cycle between backgrounds");
        hgl_rita_draw_text(10, 160 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press P to toggle between rerspective/orthographic projection");
        hgl_rita_draw_text(10, 176 + 16, 1.0f, HGL_RITA_MORTEL_RED, "Press W to toggle between wireframes");

        if (IsKeyPressed(KEY_A)) {
            frag_shader_in_use++;
            frag_shader_in_use %= (sizeof(fragment_shaders)/sizeof(fragment_shaders[0]));
        }
        if (IsKeyPressed(KEY_H)) {
            vert_shader_in_use++;
            vert_shader_in_use %= (sizeof(vertex_shaders)/sizeof(vertex_shaders[0]));
        }
        if (IsKeyPressed(KEY_S)) {
            backface_culling = !backface_culling;
        }
        if (IsKeyPressed(KEY_D)) {
            depth_testing = !depth_testing;
        }
        if (IsKeyPressed(KEY_F)) {
            frontface_winding_ccw = !frontface_winding_ccw;
        }
        if (IsKeyDown(KEY_UP) && camera_distance > 10) {
            camera_distance -= 2.0f;
        }
        if (IsKeyDown(KEY_DOWN)) {
            camera_distance += 2.0f;
        }
        if (IsKeyDown(KEY_LEFT)) {
            camera_angle -= 0.05f;
        }
        if (IsKeyDown(KEY_RIGHT)) {
            camera_angle += 0.05f;
        }
        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            camera_height -= 2.0f;
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            camera_height += 2.0f;
        }
        if (IsKeyPressed(KEY_G)) {
            filter_shader_in_use++;
            filter_shader_in_use %= (sizeof(filter_shaders)/sizeof(filter_shaders[0]));
        }
        if (IsKeyPressed(KEY_M)) {
            model_in_use++;
            model_in_use %= (sizeof(models)/sizeof(models[0]));
        }
        if (IsKeyPressed(KEY_B)) {
            background_in_use++;
            background_in_use %= 3;
        }
        if (IsKeyPressed(KEY_P)) {
            use_perspective_project = !use_perspective_project;
            if (use_perspective_project) {
                hgl_rita_use_perspective_proj(3.1415f/3.0f, (float)WIDTH/(float)HEIGHT, 2.0f, 1000.0f);
            } else {
                hgl_rita_use_orthographic_proj(-40, 40, -30, 30, 0, 1000);
            }
        }
        if (IsKeyPressed(KEY_W)) {
            wireframes = !wireframes;
            if (wireframes) {
                hgl_rita_enable(HGL_RITA_WIRE_FRAMES);
            } else {
                hgl_rita_disable(HGL_RITA_WIRE_FRAMES);
            }
        }

        hgl_rita_bind_vert_shader(vertex_shaders[vert_shader_in_use].func);
        hgl_rita_bind_frag_shader(fragment_shaders[frag_shader_in_use].func);
        if (backface_culling) hgl_rita_enable(HGL_RITA_BACKFACE_CULLING); else hgl_rita_disable(HGL_RITA_BACKFACE_CULLING);
        if (depth_testing) hgl_rita_enable(HGL_RITA_DEPTH_TESTING); else hgl_rita_disable(HGL_RITA_DEPTH_TESTING);
        if (frontface_winding_ccw) hgl_rita_use_frontface_winding_order(HGL_RITA_CCW); else hgl_rita_use_frontface_winding_order(HGL_RITA_CW);
        camera_pos = vec3_make(camera_distance*sinf(camera_angle), camera_height, camera_distance*cosf(camera_angle));
        hgl_rita_use_camera_view(camera_pos, vec3_make(0, 0, 0), vec3_make(0, 1, 0));
        //hgl_rita_use_view_matrix(view);
        
        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, frame_buffer->data.rgba8);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
        EndDrawing();

        frame_count++;
    }

    CloseWindow();
}
