#define _DEFAULT_SOURCE

#define HGL_RASTER_IMPLEMENTATION
#include "hgl_raster.h"

#define HGL_IO_IMPLEMENTATION
#include "hgl_io.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    hgl_raster_init(128, 64, HGL_RASTER_RENDERER_TERMINAL);
    //hgl_raster_init(800, 600, HGL_RASTER_RENDERER_TERMINAL);
    hgl_raster_clear(HGL_RASTER_BLACK);

#if 1
    hgl_raster_begin(HGL_RASTER_POINTS);
        hgl_raster_vertex(VERT_2D(0.0f, 0.1f, HGL_RASTER_RED));
        hgl_raster_vertex(VERT_2D(0.0f, 0.3f, HGL_RASTER_GREEN));
        hgl_raster_vertex(VERT_2D(0.0f, 0.5f, HGL_RASTER_BLUE));
    hgl_raster_end();

    hgl_raster_tform_use_pixel_coords();

    hgl_raster_begin(HGL_RASTER_POINTS);
        hgl_raster_vertex(VERT_2D(80, 32, HGL_RASTER_BLACK));
        hgl_raster_vertex(VERT_2D(80, 38, HGL_RASTER_BLACK));
        hgl_raster_vertex(VERT_2D(80, 44, HGL_RASTER_BLACK));
    hgl_raster_end();

    hgl_raster_begin(HGL_RASTER_LINES);
        hgl_raster_vertex(VERT_2D(100, 32, HGL_RASTER_RED));
        hgl_raster_vertex(VERT_2D(110, 38, HGL_RASTER_GREEN));
        hgl_raster_vertex(VERT_2D(110, 38, HGL_RASTER_GREEN));
        hgl_raster_vertex(VERT_2D(90,  44, HGL_RASTER_BLUE));
        hgl_raster_vertex(VERT_2D(90,  44, HGL_RASTER_BLUE));
        hgl_raster_vertex(VERT_2D(100, 32, HGL_RASTER_RED));
        hgl_raster_vertex(VERT_2D(100, 32, HGL_RASTER_RED));
        hgl_raster_vertex(VERT_2D(105, 0, HGL_RASTER_WHITE & 0xFFFFFF00));
    hgl_raster_end();

    hgl_raster_tform_use_normalized_coords();
    hgl_raster_begin(HGL_RASTER_LINES);
        hgl_raster_vertex(VERT_2D(0.0f, 0.5f, HGL_RASTER_RED));
        hgl_raster_vertex(VERT_2D(0.5f, 0.0f, HGL_RASTER_GREEN));
    hgl_raster_end();

    hgl_raster_tform_use_pixel_coords();
    hgl_raster_begin(HGL_RASTER_LINE_STRIP);
        hgl_raster_vertex(VERT_2D(20, 22, HGL_RASTER_RED));
        hgl_raster_vertex(VERT_2D(25, 24, HGL_RASTER_GREEN));
        hgl_raster_vertex(VERT_2D(30, 28, HGL_RASTER_GREEN));
        hgl_raster_vertex(VERT_2D(35, 37, HGL_RASTER_GREEN));
        hgl_raster_vertex(VERT_2D(37, 49, HGL_RASTER_GREEN));
        hgl_raster_vertex(VERT_2D(-100, 60, HGL_RASTER_BLACK));
    hgl_raster_end();
#endif

#if 0
    hgl_raster_tform_use_normalized_coords();
    hgl_raster_tform_view(mat4_make_translation(vec3_make(0, 0, -3.0f)));
    hgl_raster_tform_proj(mat4_make_perspective(3.14f/4, 128.0f/64.0f, 0.1f, 100.f));

    Mat4 model = mat4_make_identity();
    Vec3 axis = vec3_make(0, 1, 0);

    for (;;) {
        model = mat4_rotate(model, 0.05f, axis);
        hgl_raster_tform_model(model);
        hgl_raster_clear(HGL_RASTER_BLACK);
        hgl_raster_begin(HGL_RASTER_LINE_STRIP);
            hgl_raster_vertex(VERT_2D(-0.5f, -0.5f, HGL_RASTER_RED));
            hgl_raster_vertex(VERT_2D( 0.5f, -0.5f, HGL_RASTER_GREEN));
            hgl_raster_vertex(VERT_2D( 0.5f,  0.5f, HGL_RASTER_GREEN));
            hgl_raster_vertex(VERT_2D(-0.5f,  0.5f, HGL_RASTER_RED));
            hgl_raster_vertex(VERT_2D(-0.5f, -0.5f, HGL_RASTER_RED));
        hgl_raster_end();
        hgl_raster_render();
    }
#endif

    hgl_raster_render();

    HglImage image = {
        .data   = (uint8_t *) ctx_.canvas.pixels,
        .width  = ctx_.canvas.width,
        .height = ctx_.canvas.height,
        .format = HGL_IO_PIXEL_FORMAT_RGBA8,
    };
    hgl_io_image_write_netpbm("output.ppm", &image);
}

