
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#include "stb_image.h"

#define VERT_2D(x_, y_) ((HglRitaVertex){.pos.f = {(x_), (y_), 0.5f, 1.0f}, .color = HGL_RITA_WHITE, .uv = {0.0f, 0.0f}})
#define VERT_2D_COL(x_, y_, color_) ((HglRitaVertex){.pos.f = {(x_), (y_), 0.5f, 1.0f}, .color = (color_), .uv = {0.0f, 0.0f}})
#define VERT_3D_COL(x_, y_, z_, color_) ((HglRitaVertex){.pos.f = {(x_), (y_), (z_), 1.0f}, .color = (color_), .uv = {0.0f, 0.0f}})
#define VERT_2D_UV(x_, y_, u_, v_) ((HglRitaVertex){.pos.f = {(x_), (y_), 0.5f, 1.0f}, .color = HGL_RITA_WHITE, .uv = {(u_), (v_)}})
#define VERT_3D_UV(x_, y_, z_, u_, v_) ((HglRitaVertex){.pos.f = {(x_), (y_), (z_), 1.0f}, .color = HGL_RITA_WHITE, .uv = {(u_), (v_)}})
#define VERT_2D_COL_UV(x_, y_, color_, u_, v_) ((HglRitaVertex){.pos.f = {(x_), (y_), 0.5f, 1.0f}, .color = (color_), .uv = {(u_), (v_)}})
#define VERT_3D_COL_UV(x_, y_, z_, color_, u_, v_) ((HglRitaVertex){.pos.f = {(x_), (y_), (z_), 1.0f}, .color = (color_), .uv = {(u_), (v_)}})

typedef struct
{
    HglRitaVertShaderFunc vertex_shader;
    HglRitaFragShaderFunc fragment_shader;
    HglRitaVertexBuffer vbuf;
    HglRitaIndexBuffer ibuf;
    HglRitaTexture diffuse;
    Mat4 tform;
    HglRitaWindingOrder winding_order;
} MyModel;

static inline void draw_model(MyModel *model);
static inline MyModel load_model_from_obj(const char *filepath);
static inline HglRitaTexture load_texture(const char *filepath);

static inline void draw_model(MyModel *model)
{
    hgl_rita_use_vertex_buffer_mode(HGL_RITA_INDEXED);
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &model->vbuf);
    hgl_rita_bind_buffer(HGL_RITA_INDEX_BUFFER, &model->ibuf);
    //hgl_rita_enable(HGL_RITA_Z_CLIPPING |
    //                HGL_RITA_BACKFACE_CULLING |
    //                HGL_RITA_DEPTH_TESTING |
    //                HGL_RITA_ORDER_DEPENDENT_ALPHA_BLEND);
    hgl_rita_use_frontface_winding_order(model->winding_order);
    hgl_rita_bind_vert_shader(model->vertex_shader);
    hgl_rita_bind_frag_shader(model->fragment_shader);
    if (model->diffuse.data.rgba8 != NULL) {
        hgl_rita_bind_texture(HGL_RITA_TEX_DIFFUSE, &model->diffuse);
    }
    hgl_rita_use_model_matrix(model->tform);
    hgl_rita_draw(HGL_RITA_TRIANGLES);
}

static inline MyModel load_model_from_obj(const char *filepath)
{
    MyModel model = (MyModel) {
        .vbuf    = {0},
        .ibuf    = {0},
        .diffuse = {0},
        .tform   = mat4_make_identity(),
        .winding_order = HGL_RITA_CCW,
    };

    fastObjMesh *mesh = fast_obj_read(filepath);
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

    Vec3 *tangents = malloc(mesh->normal_count * sizeof(Vec3));
    memset(tangents, 0, mesh->normal_count * sizeof(Vec3));

    for (uint32_t i = 0; i < mesh->face_count; i++) {
        assert(mesh->face_vertices[i] == 3);
        Vec3 p[3];
        Vec2 t[3];
        for (uint32_t j = 0; j < mesh->face_vertices[i]; j++) {
            uint32_t p_idx = mesh->indices[i * 3 + j].p;
            uint32_t t_idx = mesh->indices[i * 3 + j].t;
            p[j].x = mesh->positions[p_idx*3];
            p[j].y = mesh->positions[p_idx*3 + 1];
            p[j].z = mesh->positions[p_idx*3 + 2];
            t[j].x = mesh->texcoords[t_idx*2];
            t[j].y = mesh->texcoords[t_idx*2 + 1];
        }

        float x1 = p[1].x - p[0].x;
        float x2 = p[2].x - p[0].x;
        float y1 = p[1].y - p[0].y;
        float y2 = p[2].y - p[0].y;
        float z1 = p[1].z - p[0].z;
        float z2 = p[2].z - p[0].z;

        float s1 = t[1].x - t[0].x;
        float s2 = t[2].x - t[0].x;
        float t1 = t[1].y - t[0].y;
        float t2 = t[2].y - t[0].y;

        float r = 1.0f / (s1 * t2 - s2 * t1);
        Vec3 tan = {0};
        tan.x = r * (t2 * x1 - t1 * x2);
        tan.y = r * (t2 * y1 - t1 * y2);
        tan.z = r * (t2 * z1 - t1 * z2);

        //vec3_print(p[0]);
        //vec3_print(p[1]);
        //vec3_print(p[2]);
        //vec2_print(t[0]);
        //vec2_print(t[1]);
        //vec2_print(t[2]);
        //vec3_print(tan);

        for (uint32_t j = 0; j < mesh->face_vertices[i]; j++) {
            uint32_t n_idx = mesh->indices[i * 3 + j].n;
            tangents[n_idx] = vec3_add(tangents[n_idx], tan);
        }

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
            Vec3 tan = tangents[n_idx];
            tan = vec3_normalize(vec3_sub(tan, vec3_mul_scalar(v.normal, vec3_dot(v.normal, tan))));
#ifndef HGL_RITA_SIMPLE
            v.tangent = tan;
#endif
        }
        if (uv_idx != 0) {
            v.uv.x = mesh->texcoords[uv_idx*2];
            v.uv.y = mesh->texcoords[uv_idx*2 + 1];
        }
        v.color = mtl_kd;

#if 0
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
        if (idx % 10000 == 0) printf("%d\n", idx);
        if (idx == model.vbuf.length) {
            hgl_rita_vbuf_push(&model.vbuf, v);
        }
        hgl_rita_ibuf_push(&model.ibuf, (int)idx);
#else
        /* or just shove 'em all in there lol. */
        hgl_rita_buf_push(&model.vbuf, v);
        hgl_rita_buf_push(&model.ibuf, (int)model.vbuf.length - 1);
#endif
    }

    fast_obj_destroy(mesh);
    free(tangents);

    return model;
}

static inline HglRitaTexture load_texture(const char *filepath)
{
    HglRitaTexture tex;
    int original_n_channels;
    int requested_n_channels = 4;
    stbi_set_flip_vertically_on_load(1);
    tex.data.rgba8 = (HglRitaColor *) stbi_load(filepath, &tex.width, &tex.height,
                                                &original_n_channels,
                                                requested_n_channels);
    assert(tex.data.rgba8 != 0);
    assert(tex.width != 0);
    assert(tex.height != 0);
    tex.stride = tex.width;
    tex.format = HGL_RITA_RGBA8;
    return tex;
}
