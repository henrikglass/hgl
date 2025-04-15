#define _DEFAULT_SOURCE

#define HGL_MEMDBG_IMPLEMENTATION
#include "hgl_memdbg.h"

#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"
#include "rita_helpers.h"

#include "raylib.h"

#include "stb_image.h"

#include <stdio.h>

static int frame_count = 0;
static HglRitaVertex wobbly_vert_shader(const HglRitaContext *ctx, const HglRitaVertex *in)
{
    HglRitaVertex out;
    Vec4 v;

    /* get local space vertex pos and extend to 4D */
    v = in->pos;
    v.w = 1.0f;

    v.x += 0.03*sinf(10*in->pos.y + frame_count*0.1);
    v.z += 0.03*cosf(10*in->pos.y + frame_count*0.1);
    v.y += 0.05*sinf(3.14f + 2*in->pos.y + frame_count*0.1);

    /* local space --> world space -> view space -> clip space */
    v = mat4_mul_vec4(ctx->tform.mvp, v);

    out.pos     = v;
    out.normal  = in->normal;
    out.uv      = in->uv;
    out.color   = in->color;

    return out;
}

void draw_model(MyModel *model, Mat4 proj, Mat4 view)
{
    hgl_rita_enable(HGL_RITA_Z_CLIPPING);
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);
    hgl_rita_enable(HGL_RITA_BACKFACE_CULLING);
    hgl_rita_enable(HGL_RITA_DEPTH_TESTING);

    model->tform = mat4_rotate(model->tform, 0.02f, vec3_normalize(vec3_make(0.0, 1, 0.0)));

    hgl_rita_use_view_matrix(view);
    hgl_rita_use_proj_matrix(proj);
    use_model(model);
    hgl_rita_bind_vert_shader((HglRitaVertShaderFunc) wobbly_vert_shader);

    hgl_rita_use_frontface_winding_order(model->winding);
    hgl_rita_draw(HGL_RITA_TRIANGLES);
    hgl_rita_bind_vert_shader(NULL);
}


//void parse_obj_data(HglRitaVertexBuffer *vbuf, HglRitaIndexBuffer *ibuf, const char *filepath);

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    #define WIDTH 512
    #define HEIGHT 512

    /* rita stuff */
    hgl_rita_init();
    hgl_rita_use_clear_color(HGL_RITA_MORTEL_BLACK);
    //hgl_rita_enable(HGL_RITA_WIRE_FRAMES);

    HglRitaTexture large_color = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8); 
    HglRitaTexture small_color = hgl_rita_texture_make(64, 64, HGL_RITA_RGBA8);
    HglRitaTexture large_depth = hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_R32);
    HglRitaTexture small_depth = hgl_rita_texture_make(64, 64, HGL_RITA_R32);

    /* raylib stuff */
    InitWindow(2*WIDTH, HEIGHT, "HglRita test");
    Image color_image = (Image) {
        .data    = large_color.data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };

    Image depth_image = (Image) {
        .data    = large_depth.data.r32,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R32,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);
    Texture2D depth_tex = LoadTextureFromImage(depth_image);

    Mat4 model  = mat4_translate(MAT4_IDENTITY, vec3_make(0, 0.5, -10));
    model = mat4_rotate(model, 3.14f, vec3_make(1, 0, 0));
    Mat4 model2 = mat4_translate(MAT4_IDENTITY, vec3_make(0, 0, -10));
    Mat4 view   = mat4_make_translation(vec3_make(0, 0, 3.0f));
    Mat4 proj   = mat4_make_perspective(3.14f/4, (float)(WIDTH)/(float)(HEIGHT), 1.0f, 100.0f);
    hgl_rita_use_frontface_winding_order(HGL_RITA_CCW);

    HglRitaTexture tex;
    stbi_set_flip_vertically_on_load(0);
    tex.data.rgba8 = (HglRitaColor *) stbi_load("assets/box32x32.png",&tex.height, &tex.width, NULL, 4);
    tex.format = HGL_RITA_RGBA8;
    tex.stride = tex.width;

    MyModel penger = load_model_from_obj("assets/penger.obj");
    penger.winding = HGL_RITA_CCW;
    //penger.tform = mat4_scale(penger.tform, vec3_make(0.2, 0.2, 0.2));
    penger.tform = mat4_translate(penger.tform, vec3_make(0, -0.5, -6));

    (void) model;
    (void) model2;
    (void) view;
    (void) proj;

    SetTargetFPS(60);
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        frame_count++;

        HGL_PROFILE_MEASURE_NAMED("hgl_rita draw 1",
            hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &small_color);
            hgl_rita_bind_texture(HGL_RITA_TEX_DEPTH_BUFFER, &small_depth);
            hgl_rita_use_viewport(64, 64);
            hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
            draw_model(&penger, proj, view);
        );

#if 1
        HGL_PROFILE_MEASURE_NAMED("hgl_rita draw 2",
            hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, &large_color);
            hgl_rita_bind_texture(HGL_RITA_TEX_DEPTH_BUFFER, &large_depth);
            hgl_rita_use_viewport(512, 512);
            hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
            for (int y = 0; y < 512/64; y++) {
                for (int x = 0; x < 512/64; x++) {
                    if (x % 2 && y % 2)
                        hgl_rita_blit_framebuffer(64*x, 64*y, 64, 64, small_color, HGL_RITA_REPLACE, HGL_RITA_EVERYWHERE, HGL_RITA_BOXCOORD);
                    if (x % 2 == 0 && y % 2 == 0)
                        hgl_rita_blit_framebuffer(64*x, 64*y, 64, 64, small_color, HGL_RITA_REPLACE, HGL_RITA_EVERYWHERE, HGL_RITA_BOXCOORD);
                }
            } 
            draw_model(&penger, proj, view);
        );
#endif
#if 0
        HGL_PROFILE_MEASURE_NAMED("hgl_rita draw 2",
            hgl_rita_bind(HGL_RITA_FRAME_BUFFER, &fb_large);
            hgl_rita_use_viewport(512, 512);
            hgl_rita_clear(HGL_RITA_COLOR | HGL_RITA_DEPTH);
            hgl_rita_blit(0, 0, WIDTH, HEIGHT, &fb_small.color_attachment, HGL_RITA_REPLACE, HGL_RITA_EVERYWHERE, HGL_RITA_BOXCOORD);
        );
#endif

        /* raylib stuff */
        UpdateTexture(color_tex, color_image.data);
        UpdateTexture(depth_tex, depth_image.data);
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(color_tex, 0, 0, WHITE);
            DrawTexture(depth_tex, WIDTH, 0, WHITE);
            DrawFPS(128, 10);
        EndDrawing();

        hgl_profile_report(HGL_PROFILE_LAST);

        //render_terminal(0);
    }


    CloseWindow();
    hgl_profile_report(HGL_PROFILE_EVERYTHING);
    hgl_profile_cleanup();

    free_model(&penger);

    hgl_memdbg_report();

    return 0;
}

void use_model(MyModel *model)
{
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &model->vbuf);
    hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &model->ibuf);
    hgl_rita_use_model_matrix(model->tform);
    if (model->texture.data.rgba8 != NULL) {
        hgl_rita_bind_texture(HGL_RITA_TEX_DIFFUSE, &model->texture);
    } 
}

MyModel load_model_from_obj(const char *filepath)
{
    fastObjMesh *mesh = fast_obj_read(filepath);

    MyModel model = (MyModel) {
        //.vbuf    = hgl_rita_vbuf_make(mesh->position_count),
        //.ibuf    = hgl_rita_ibuf_make(mesh->index_count),
        .vbuf    = hgl_rita_vbuf_make(10),
        .ibuf    = hgl_rita_ibuf_make(10),
        .texture = {0},
        .tform   = mat4_make_identity(),
    };

    assert(mesh->texture_count > 0);
    HglRitaColor mtl_kd = HGL_RITA_WHITE;
    if (mesh->material_count > 0) {
        mtl_kd = (HglRitaColor) {
            .r = mesh->materials[mesh->material_count - 1].Kd[0] * 255,
            .g = mesh->materials[mesh->material_count - 1].Kd[1] * 255,
            .b = mesh->materials[mesh->material_count - 1].Kd[2] * 255,
            .a = 255,
        };
    } 
    stbi_set_flip_vertically_on_load(1);
    int original_n_channels;
    int requested_n_channels = 4;
    model.texture.data.rgba8 = (HglRitaColor *) stbi_load(mesh->textures[mesh->texture_count - 1].path, 
                                                          &model.texture.width, 
                                                          &model.texture.height, 
                                                          &original_n_channels, 
                                                          requested_n_channels);
    model.texture.format = HGL_RITA_RGBA8;
    model.texture.stride = model.texture.width;
    if (model.texture.data.rgba8 == NULL) {
        printf("Failed to load texture.\n");
    }
    for (uint32_t i = 0; i < mesh->index_count; i++) {
        /* construct vertex `v` */
        HglRitaVertex v = {0};
        uint32_t pos_idx = mesh->indices[i].p;
        uint32_t uv_idx  = mesh->indices[i].t;
        uint32_t n_idx   = mesh->indices[i].n;
        if (pos_idx != 0) {
            v.pos.x = mesh->positions[pos_idx*3];
            v.pos.y = mesh->positions[pos_idx*3 + 1];
            v.pos.z = mesh->positions[pos_idx*3 + 2];
            v.pos.w = 0.0f;
        }
        if (n_idx != 0) {
            v.normal.x = mesh->normals[n_idx*3];
            v.normal.y = mesh->normals[n_idx*3 + 1];
            v.normal.z = mesh->normals[n_idx*3 + 2];
        }
        if (uv_idx != 0) {
            v.uv.x = mesh->texcoords[uv_idx*2];
            v.uv.y = mesh->texcoords[uv_idx*2 + 1];
        }
        v.color = mtl_kd;

        /* 
         * Insert `v` into `vbuf` if there's not already an identical
         * vertex in it. O(n^2). Could be better with a hash table/set.
         */
        int idx;
        for (idx = 0; idx < model.vbuf.length; idx++) {
            if (hgl_rita_vertex_eq(v, model.vbuf.arr[idx])) {
                break;
            }
        }
        if (idx == model.vbuf.length) {
            hgl_rita_vbuf_push(&model.vbuf, v);
        }
        hgl_rita_ibuf_push(&model.ibuf, (int)idx);
    }

    printf("Computed:\n");
    printf("#verts = %d\n", model.vbuf.length);
    printf("#indices = %d\n", model.ibuf.length);
    fast_obj_destroy(mesh);

    return model;
}

void free_model(MyModel *model)
{
    hgl_rita_vbuf_destroy(&model->vbuf);
    hgl_rita_ibuf_destroy(&model->ibuf);
}
