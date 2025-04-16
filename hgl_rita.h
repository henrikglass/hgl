
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2025 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_rita.h is a multi-threaded (tiled) CPU-rasterizer and general-purpose graphics library
 * in a single C header-file (not counting hglm.h & hglm_aliases.h).
 *
 * hgl_rita.h features:
 *
 *     - An OpenGL-esque render pipeline, with support for optional user-programmable
 *       vertex and fragment shaders.
 *     - A set of two fixed vertex specifications, DEFAULT and SIMPLE. The DEFAULT vertex
 *       specification contains most of the commonly used vertex attributes. The SIMPLE 
 *       vertex specification is a subset of the DEFAULT vertex specification:
 *          - pos           : vertex position vector
 *          - normal        : vertex normal vector
 *          - tangent       : vertex tangent vector (Omitted in the SIMPLE vertex specification)
 *          - uv            : vertex texture coordinate
 *          - color         : vertex color
 *     - A set of two fixed fragment specifications, DEFAULT and SIMPLE. The DEFAULT fragment
 *       specification contains most of the commonly used fragment attributes. The SIMPLE 
 *       fragment specification is a subset of the DEFAULT fragment specification:
 *          - world_pos     : fragment world position vector (Omitted in the SIMPLE vertex specification)
 *          - world_normal  : fragment world normal vector
 *          - world_tangent : fragment world tangent vector (Omitted in the SIMPLE vertex specification)
 *          - uv            : fragment texture coordinate
 *          - x             : fragment screen space x-coordinate
 *          - y             : fragment screen space y-coordinate
 *          - inv_z         : fragment depth
 *     - A 3D-capable fixed function pipeline (if no custom shaders are used). The user
 *       must simply set: `hgl_rita_use_model_matrix()`, `hgl_rita_use_view_matrix()`,
 *       and `hgl_rita_use_proj_matrix()`.
 *     - A powerful blit function for applying images and shaders directly to the
 *       frame buffer.
 *     - Simple text rendering with a shitty bitmap font.
 *     - A handful of standalone functions for dealing with vertex- and index buffers,
 *       colors, textures, vertex and fragment interpolation, etc.
 *     - Low-level customization of the parallel rendering engine. The user may specify,
 *       at compile time, details such as tile dimensions, tile operation-queue size,
 *       and decide whether vertex processing will be done serially or in parallel.
 *
 * The hgl_rita.h rendering engine is based on a tiled (a.k.a sort middle) rasterizer architecture. By default,
 * geometry processing (vertex shading) is done just-in-time by whichever thread issued the draw call
 * (`hgl_rita_draw()`); i.e. vertices are processed as they are needed immediately before primitives are
 * dispatched to the tile threads. However, if HGL_RITA_PARALLEL_VERTEX_PROCESSING is defined, geometry
 * processing is done up-front and in parallel. HGL_RITA_PARALLEL_VERTEX_PROCESSING typically yields better
 * performance when drawing large meshes (at least on my machine).
 *
 * The frame- and depth buffers are divided into a number of 2D tiles. Each tile has its own thread,
 * conveniently referred to as a `tile thread`. Each tile thread is soley responsible for rendering into
 * its assigned area of the frame and depth buffer, with one exception (see below). Each tile has its own
 * operation queue (op_queue). When a primitive (point, line, triangle) is "dispatched", an operation
 * is placed onto the end of the operation queues of all tiles intersecting it. operations are processed
 * by the tile thread in-order. For regular draw calls (OP_RASTER_POINT, OP_RASTERIZE_LINE, OP_RASTERIZE_TRI)
 * the tile threads performs rasterization, fragment shading, and subsequent writing to the frame and depth
 * buffer. Tile threads are also used to parallelize blit operations issued via `hgl_rita_blit()` (OP_BLIT).
 * Tile threads may also be used for up-front vertex processing iff HGL_RITA_PARALLEL_VERTEX_PROCESSING
 * is defined (OP_PROCESS_VERTICES). To ensure that all tile threads have completed their work, the user
 * must call `hgl_rita_finish()`. `hgl_rita_finish()` will block until all tile op-queues are empty and
 * all tile threads themselves are blocked, waiting for new operations to arrive on the op-queue. The
 * only operation on the frame buffer which is not parallelized is `hgl_rita_draw_text()`.
 * `hgl_rita_draw_text()` will indirectly call `hgl_rita_finish()` before operating on the frame buffer
 * to ensure exclusive access to it.
 *
 * By default, each tile is 256 pixels wide, 64 pixels high and has an op-queue with a capacity of 256.
 * These settings tend to give consistently decent performance for most workloads on my machine. The
 * optimum settings, however, may differ depending on workload and on your machine. These values can
 * be changed at compile-time by defining the following macros before including hgl_rita.h:
 *
 *     HGL_RITA_TILE_SIZE_X
 *     HGL_RITA_TILE_SIZE_Y
 *     HGL_RITA_TILE_OP_QUEUE_CAPACITY
 *
 * hgl_rita.h includes a handful of presets for these settings, as well as HGL_RITA_PARALLEL_VERTEX_PROCESSING
 * which may be defined instead, before including hgl_rita.h:
 *
 *     HGL_RITA_PRESET_128X64X64_SERIAL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_194X64X128_SERIAL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_256X64X256_SERIAL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_256X64X2048_SERIAL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_256X64X4096_SERIAL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_128X64X64_PARALLEL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_194X64X128_PARALLEL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_256X64X256_PARALLEL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_256X64X2048_PARALLEL_VERTEX_PROCESSING
 *     HGL_RITA_PRESET_256X64X4096_PARALLEL_VERTEX_PROCESSING
 *
 * The vertex and fragment specifications may be changed from DEFAULT to SIMPLE by defining:
 *
 *     HGL_RITA_SIMPLE
 *
 * The SIMPLE vertex and fragment specifications omit most of the attributes typically used when 
 * doing 3D rendering with lighting. The SIMPLE vertex and fragment specification may be used when
 * doing simpler 2D rendering, or when attributes such as tangent/bitangent vectors aren't needed,
 * to gain a tiny bit of performance.
 *
 * USAGE:
 *
 * Import hgl_rita.h like this:
 *
 *     #define HGL_RITA_IMPLEMENTATION
 *     #include "hgl_rita.h"
 *     #include "hgl_rita_shaders.h" // optional
 *
 *
 * EXAMPLES:
 *
 * See the examples/ directory.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_RITA_H
#define HGL_RITA_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hglm.h"
#include "hglm_aliases.h"

#include <stdbool.h>
#include <stdarg.h>

/*--- Public macros ---------------------------------------------------------------------*/

#define HGL_RITA_TRANSPARENT        (HglRitaColor){.r = 0x00, .g = 0x00, .b = 0x00, .a = 0x00}
#define HGL_RITA_WHITE              (HglRitaColor){.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}
#define HGL_RITA_BLACK              (HglRitaColor){.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF}
#define HGL_RITA_RED                (HglRitaColor){.r = 0xFF, .g = 0x00, .b = 0x00, .a = 0xFF}
#define HGL_RITA_GREEN              (HglRitaColor){.r = 0x00, .g = 0xFF, .b = 0x00, .a = 0xFF}
#define HGL_RITA_BLUE               (HglRitaColor){.r = 0x00, .g = 0x00, .b = 0xFF, .a = 0xFF}
#define HGL_RITA_DARK_RED           (HglRitaColor){.r = 0x7F, .g = 0x00, .b = 0x00, .a = 0xFF}
#define HGL_RITA_DARK_GREEN         (HglRitaColor){.r = 0x00, .g = 0x7F, .b = 0x00, .a = 0xFF}
#define HGL_RITA_DARK_BLUE          (HglRitaColor){.r = 0x00, .g = 0x00, .b = 0x7F, .a = 0xFF}
#define HGL_RITA_MAGENTA            (HglRitaColor){.r = 0xFF, .g = 0x00, .b = 0xFF, .a = 0xFF}
#define HGL_RITA_CYAN               (HglRitaColor){.r = 0x00, .g = 0xFF, .b = 0xFF, .a = 0xFF}
#define HGL_RITA_YELLOW             (HglRitaColor){.r = 0xFF, .g = 0xFF, .b = 0x00, .a = 0xFF}
#define HGL_RITA_DARK_GRAY          (HglRitaColor){.r = 0x40, .g = 0x40, .b = 0x40, .a = 0xFF}
#define HGL_RITA_GRAY               (HglRitaColor){.r = 0x80, .g = 0x80, .b = 0x80, .a = 0xFF}
#define HGL_RITA_LIGHT_GRAY         (HglRitaColor){.r = 0xC0, .g = 0xC0, .b = 0xC0, .a = 0xFF}
#define HGL_RITA_MORTEL_BLACK       (HglRitaColor){.r = 0x1E, .g = 0x1E, .b = 0x1E, .a = 0xFF}
#define HGL_RITA_MORTEL_WHITE       (HglRitaColor){.r = 0xE1, .g = 0xE1, .b = 0xE1, .a = 0xFF}
#define HGL_RITA_MORTEL_RED         (HglRitaColor){.r = 0xE1, .g = 0x1E, .b = 0x1E, .a = 0xFF}
#define HGL_RITA_MORTEL_GREEN       (HglRitaColor){.r = 0x1E, .g = 0xE1, .b = 0x1E, .a = 0xFF}
#define HGL_RITA_MORTEL_BLUE        (HglRitaColor){.r = 0x1E, .g = 0x1E, .b = 0xE1, .a = 0xFF}
#define HGL_RITA_MORTEL_MAGENTA     (HglRitaColor){.r = 0xE1, .g = 0x1E, .b = 0xE1, .a = 0xFF}
#define HGL_RITA_MORTEL_CYAN        (HglRitaColor){.r = 0x1E, .g = 0xE1, .b = 0xE1, .a = 0xFF}
#define HGL_RITA_MORTEL_YELLOW      (HglRitaColor){.r = 0xE1, .g = 0xE1, .b = 0x1E, .a = 0xFF}

#ifndef HGL_RITA_MAX_N_TILES
#  define HGL_RITA_MAX_N_TILES               1024
#endif

#ifdef HGL_RITA_PRESET_128X64X64_SERIAL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                128
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY      64
#endif

#ifdef HGL_RITA_PRESET_192X64X128_SERIAL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                192
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY     128
#endif

#ifdef HGL_RITA_PRESET_256X64X256_SERIAL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                256
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY     256
#endif

#ifdef HGL_RITA_PRESET_256X64X2048_SERIAL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                256
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY    2048
#endif

#ifdef HGL_RITA_PRESET_256X64X4096_SERIAL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                256
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY    4096
#endif

#ifdef HGL_RITA_PRESET_128X64X64_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                128
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY      64
#endif

#ifdef HGL_RITA_PRESET_192X64X128_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                192
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY     128
#endif

#ifdef HGL_RITA_PRESET_256X64X256_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                256
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY     256
#endif

#ifdef HGL_RITA_PRESET_256X64X2048_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                256
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY    2048
#endif

#ifdef HGL_RITA_PRESET_256X64X4096_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_PARALLEL_VERTEX_PROCESSING
#  define HGL_RITA_TILE_SIZE_X                256
#  define HGL_RITA_TILE_SIZE_Y                 64
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY    4096
#endif


#ifndef HGL_RITA_TILE_SIZE_X
#  define HGL_RITA_TILE_SIZE_X                256
#endif

#ifndef HGL_RITA_TILE_SIZE_Y
#  define HGL_RITA_TILE_SIZE_Y                 64
#endif

#ifndef HGL_RITA_TILE_OP_QUEUE_CAPACITY
#  define HGL_RITA_TILE_OP_QUEUE_CAPACITY     256
#endif

#define HGL_RITA_TEXT_BUFFER_MAX_SIZE 4096

#if !defined(HGL_RITA_ALLOC) && \
    !defined(HGL_RITA_REALLOC) && \
    !defined(HGL_RITA_FREE)
#include <stdlib.h>
#define HGL_RITA_ALLOC    malloc
#define HGL_RITA_REALLOC  realloc
#define HGL_RITA_FREE     free
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

/*---------------------------------------------------------------------------------------*/
/*--- Dynamic buffer macro implementation -----------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

#define HglRitaDynamicBuffer(T) \
    struct {                    \
        T *arr;                 \
        int length;             \
        int capacity;           \
    }

#define hgl_rita_buf_reserve(da, new_capacity)                                            \
    do {                                                                                  \
        int new_capacity_ = new_capacity;                                                 \
        if ((da)->capacity >= (new_capacity_)) break;                                     \
        new_capacity_--;                                                                  \
        new_capacity_ |= new_capacity_ >> 1;                                              \
        new_capacity_ |= new_capacity_ >> 2;                                              \
        new_capacity_ |= new_capacity_ >> 4;                                              \
        new_capacity_ |= new_capacity_ >> 8;                                              \
        new_capacity_ |= new_capacity_ >> 16;                                             \
        new_capacity_++;                                                                  \
        (da)->capacity = new_capacity_;                                                   \
        (da)->arr = HGL_RITA_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr));     \
    } while (0)

#define hgl_rita_buf_reserve_exact(da, new_capacity)                                      \
    do {                                                                                  \
        int new_capacity_ = new_capacity;                                                 \
        if ((da)->capacity >= (new_capacity_)) break;                                     \
        (da)->capacity = new_capacity_;                                                   \
        (da)->arr = HGL_RITA_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr));     \
    } while (0)

#define hgl_rita_buf_push(da, ...)                                                        \
    do {                                                                                  \
        if ((da)->arr == NULL) {                                                          \
            (da)->length = 0;                                                             \
            (da)->capacity = 16;                                                          \
            (da)->arr = HGL_RITA_ALLOC((da)->capacity * sizeof(*(da)->arr));              \
        }                                                                                 \
        if ((da)->capacity < ((da)->length + 1)) {                                        \
            (da)->capacity *= 2;                                                          \
            (da)->arr = HGL_RITA_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr)); \
        }                                                                                 \
        (da)->arr[(da)->length++] = (__VA_ARGS__);                                        \
    } while (0)

#define hgl_rita_buf_pop(da) ((da)->arr[--(da)->length])
#define hgl_rita_buf_clear(da) ((da)->length = 0)

#define hgl_rita_buf_destroy(da)                                                          \
    do {                                                                                  \
        if ((da)->arr != NULL) {                                                          \
            HGL_RITA_FREE((da)->arr);                                                     \
        }                                                                                 \
    } while (0)

/*---------------------------------------------------------------------------------------*/
/*--- Thread queue macro implementation -------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

#define HGL_RITA_TQ_ARR_DECL_ASSERT(T, N) T arr[(N > 1 && N <= UINT16_MAX) ? N : -1] // Assert that N is in the range [2, 2^16]

#define HglRitaThreadQueue(T, N)                                                          \
    struct                                                                                \
    {                                                                                     \
        HGL_RITA_TQ_ARR_DECL_ASSERT(T, N);                                                \
        pthread_mutex_t mutex;                                                            \
        pthread_cond_t cvar_writable;                                                     \
        pthread_cond_t cvar_readable;                                                     \
        uint16_t wp;                                                                      \
        uint16_t rp;                                                                      \
        _Atomic int n_idle;                                                               \
    }

#define hgl_rita_queue_capacity(q) (sizeof((q)->arr) / sizeof((q)->arr[0]))
#define hgl_rita_queue_is_empty(q) ((q)->rp == (q)->wp)
#define hgl_rita_queue_is_full(q) ((((q)->wp + 1) & (hgl_rita_queue_capacity(q) - 1)) == (q)->rp)

#define hgl_rita_queue_init(q)                                                            \
    do {                                                                                  \
        (q)->wp = 0;                                                                      \
        (q)->rp = 0;                                                                      \
        (q)->n_idle = 0;                                                                  \
        assert(0 == pthread_mutex_init(&(q)->mutex, NULL));                               \
        assert(0 == pthread_cond_init(&(q)->cvar_writable, NULL));                        \
        assert(0 == pthread_cond_init(&(q)->cvar_readable, NULL));                        \
    } while (0)

#define hgl_rita_queue_destroy(q)                                                         \
    do {                                                                                  \
        pthread_mutex_lock(&(q)->mutex);                                                  \
        pthread_mutex_unlock(&(q)->mutex);                                                \
        assert(0 == pthread_mutex_destroy(&(q)->mutex));                                  \
        assert(0 == pthread_cond_destroy(&(q)->cvar_writable));                           \
        assert(0 == pthread_cond_destroy(&(q)->cvar_readable));                           \
    } while (0)

#define hgl_rita_queue_push(q, item)                                                      \
    do {                                                                                  \
        pthread_mutex_lock(&(q)->mutex);                                                  \
        while(hgl_rita_queue_is_full(q)) {                                                \
            pthread_cond_wait(&(q)->cvar_writable, &(q)->mutex);                          \
        }                                                                                 \
        (q)->arr[(q)->wp] = item;                                                         \
        (q)->wp = ((q)->wp + 1) & (hgl_rita_queue_capacity(q) - 1);                       \
        pthread_cond_signal(&(q)->cvar_readable);                                         \
        pthread_mutex_unlock(&(q)->mutex);                                                \
    } while (0)

#define hgl_rita_queue_pop(q, T)                                                          \
    ({                                                                                    \
        pthread_mutex_lock(&(q)->mutex);                                                  \
        while(hgl_rita_queue_is_empty(q)) {                                               \
            (q)->n_idle++;                                                                \
            pthread_cond_wait(&(q)->cvar_readable, &(q)->mutex);                          \
            (q)->n_idle--;                                                                \
        }                                                                                 \
        T item = (q)->arr[(q)->rp];                                                       \
        (q)->rp = ((q)->rp + 1) & (hgl_rita_queue_capacity(q) - 1);                       \
        pthread_cond_signal(&(q)->cvar_writable);                                         \
        pthread_mutex_unlock(&(q)->mutex);                                                \
        item;                                                                             \
    })

#define hgl_rita_queue_wait_until_empty(q)                                                \
    do {                                                                                  \
        pthread_mutex_lock(&(q)->mutex);                                                  \
        while(!hgl_rita_queue_is_empty(q)) {                                              \
            pthread_cond_wait(&(q)->cvar_writable, &(q)->mutex);                          \
        }                                                                                 \
        pthread_mutex_unlock(&(q)->mutex);                                                \
    } while (0)

#define hgl_rita_queue_wait_until_idle(q, n)                                              \
    do {                                                                                  \
        hgl_rita_queue_wait_until_empty(q);                                               \
        while((q)->n_idle < n);                                                           \
    } while (0)

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    HGL_RITA_VERTEX_BUFFER,
    HGL_RITA_INDEX_BUFFER,
} HglRitaBuffer;

typedef enum
{
    HGL_RITA_COLOR = 1,
    HGL_RITA_DEPTH = 2,
} HglRitaFramebufferAttachment;

typedef enum
{
    HGL_RITA_BACKFACE_CULLING            = (1 << 0),
    HGL_RITA_DEPTH_TESTING               = (1 << 1),
    HGL_RITA_ORDER_DEPENDENT_ALPHA_BLEND = (1 << 2),
    HGL_RITA_Z_CLIPPING                  = (1 << 3),
    HGL_RITA_DEPTH_BUFFER_WRITING        = (1 << 4),
    HGL_RITA_WIRE_FRAMES                 = (1 << 5),
} HglRitaOpt;

typedef enum
{
    HGL_RITA_TEX_DEFAULT      = 0,
    HGL_RITA_TEX_DIFFUSE      = 0,
    HGL_RITA_TEX_SPECULAR     = 1,
    HGL_RITA_TEX_ROUGHNESS    = 2,
    HGL_RITA_TEX_GLOSS        = 2,
    HGL_RITA_TEX_EMISSIVE     = 3,
    HGL_RITA_TEX_NORMAL       = 4,
    HGL_RITA_TEX_DISPLACEMENT = 5,
    HGL_RITA_TEX_FRAME_BUFFER = 6,
    HGL_RITA_TEX_DEPTH_BUFFER = 7,
    HGL_RITA_N_TEXTURE_UNITS
} HglRitaTexUnit;

typedef enum
{
    HGL_RITA_ARRAY,
    HGL_RITA_INDEXED,
} HglRitaVertexBufferMode;

typedef enum {
    HGL_RITA_CCW,
    HGL_RITA_CW,
} HglRitaWindingOrder;

typedef enum
{
    HGL_RITA_LINES,
    HGL_RITA_LINE_STRIP,
    HGL_RITA_POINTS,
    HGL_RITA_TRIANGLES,
    HGL_RITA_TRIANGLE_STRIP,
    HGL_RITA_TRIANGLE_FAN,
} HglRitaPrimitiveMode;

typedef enum
{
    HGL_RITA_NEAREST,
    HGL_RITA_BILINEAR,
} HglRitaTextureFilter;

typedef enum
{
    HGL_RITA_NO_WRAPPING,
    HGL_RITA_CLAMP,
    HGL_RITA_REPEAT,
} HglRitaTextureWrapping;

typedef enum
{
    HGL_RITA_REPLACE,
    HGL_RITA_REPLACE_SKIP_ALPHA,
    HGL_RITA_ALPHA,
    HGL_RITA_ONE_MINUS_ALPHA,
    HGL_RITA_ADD,
    HGL_RITA_SUBTRACT,
    HGL_RITA_SUBTRACT_SKIP_ALPHA,
    HGL_RITA_MULTIPLY,
} HglRitaBlendMethod;

typedef enum
{
    HGL_RITA_EVERYWHERE,
    HGL_RITA_CLEAR_COLOR,
    HGL_RITA_NON_CLEAR_COLOR,
    HGL_RITA_DEPTH_INF,
    HGL_RITA_DEPTH_NON_INF,
} HglRitaBlitFBMask;

typedef enum
{
    HGL_RITA_BOXCOORD,
    HGL_RITA_SCREENCOORD,
    HGL_RITA_VIEW_DIR_RECTILINEAR,
    HGL_RITA_VIEW_DIR_CUBEMAP,
    HGL_RITA_SHADER,
} HglRitaBlitFBSampler;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} HglRitaColor;

typedef struct
{
#ifndef HGL_RITA_SIMPLE
    Vec3 world_pos;
    Vec3 world_tangent;
#endif
    Vec3 world_normal;
    Vec2 uv;
    HglRitaColor color;
    int x;
    int y;
    float inv_z;
    bool clipping; // TODO figure something else out?
} HglRitaFragment;

typedef struct HglRitaVertex
{
    Vec4 pos;
    Vec3 normal;
#ifndef HGL_RITA_SIMPLE
    Vec3 tangent;
#endif
    Vec2 uv;
    HglRitaColor color;
} HglRitaVertex;

typedef HglRitaDynamicBuffer(HglRitaVertex) HglRitaVertexBuffer;
typedef HglRitaDynamicBuffer(int) HglRitaIndexBuffer;
typedef HglRitaDynamicBuffer(HglRitaFragment) HglRitaFragmentBuffer;

typedef struct
{
    uint8_t bitmap[6];
    uint8_t stride;
    uint8_t vertical_offset;
} HglRitaGlyph;

typedef enum
{
    HGL_RITA_RGBA8,
    HGL_RITA_R32,
} HglRitaPixelFormat;

typedef struct
{
    HglRitaPixelFormat format;
    union {
        HglRitaColor *rgba8;
        float *r32;
    } data;
    int width;
    int height;
    int stride;
} HglRitaTexture;

typedef struct
{
    int min_x;
    int min_y;
    int max_x;
    int max_y;
} HglRitaAABB;

typedef struct
{
    HglRitaFragment f0;
    HglRitaFragment f1;
    HglRitaFragment f2;
} HglRitaTriangle;

typedef struct
{
    HglRitaFragment f0;
    HglRitaFragment f1;
} HglRitaLine;

typedef struct
{
    HglRitaFragment f0;
} HglRitaPoint;

typedef struct
{
    int start_idx;
    int end_idx;
} HglRitaVertexBufferSegment;

struct HglRitaContext;

typedef HglRitaVertex (*HglRitaVertShaderFunc)(const struct HglRitaContext *ctx, const HglRitaVertex *in);
typedef HglRitaColor (*HglRitaFragShaderFunc)(const struct HglRitaContext *ctx, const HglRitaFragment *in);

typedef struct
{
    HglRitaAABB aabb;
    HglRitaTexture *texture;
    HglRitaBlendMethod blend_method;
    HglRitaBlitFBMask mask;
    HglRitaBlitFBSampler sampler;
    HglRitaFragShaderFunc shader;
} HglRitaBlitInfo;

typedef enum
{
    HGL_RITA_OP_RASTERIZE_TRIANGLE,
    HGL_RITA_OP_RASTERIZE_LINE,
    HGL_RITA_OP_RASTERIZE_POINT,
    HGL_RITA_OP_PROCESS_VBUF_SEGMENT,
    HGL_RITA_OP_BLIT,
    HGL_RITA_OP_TERMINATE,
} HglRitaTileOpKind;

typedef struct
{
    union {
        HglRitaTriangle triangle;
        HglRitaLine line;
        HglRitaPoint point;
        HglRitaVertexBufferSegment vbuf_segment;
        HglRitaBlitInfo blit_info;
    };
    HglRitaTileOpKind kind;
} HglRitaTileOp;

typedef HglRitaThreadQueue(HglRitaTileOp, HGL_RITA_TILE_OP_QUEUE_CAPACITY) HglRitaTileOpQueue;

typedef struct
{
    pthread_t thread;
    HglRitaTileOpQueue op_queue;
    HglRitaAABB aabb;
} HglRitaTile;

typedef struct HglRitaContext
{
    struct {
        HglRitaVertShaderFunc vert;
        HglRitaFragShaderFunc frag;
    } shaders;

    struct {
        HglRitaVertexBufferMode  mode;
        HglRitaVertexBuffer     *vbuf;
        HglRitaIndexBuffer      *ibuf;
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
        HglRitaFragmentBuffer    fbuf;
#endif
        int counter;
    } vertices;

    struct {
        HglRitaWindingOrder frontface_winding;
        HglRitaColor clear_color;
        HglRitaTextureFilter texture_filter;
        HglRitaTextureWrapping texture_wrapping;
        bool backface_culling_enabled;
        bool depth_test_enabled;
        bool order_dependent_alpha_blending_enabled;
        bool z_clipping_enabled;
        bool depth_buffer_writing_enabled;
        bool draw_wire_frames;
    } opts;

    struct {
        Mat4 model;
        Mat4 view;
        Mat4 proj;
        Mat4 mv;
        Mat4 mvp;
        Mat4 viewport;
        Mat3 normals;
        Mat3 iview;
        struct {
            Vec3 position;
            Vec3 target;
            Vec3 up;
            float fov;
            float aspect;
            float znear;
            float zfar;
        } camera;
    } tform;

    HglRitaTexture *tex_unit[HGL_RITA_N_TEXTURE_UNITS];

    struct {
        HglRitaTile tile[HGL_RITA_MAX_N_TILES];
        int n_tiles;
        int n_tile_cols;
        int n_tile_rows;
        int n_procs;
    } renderer;

} HglRitaContext;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

/* General */
static inline void hgl_rita_init(void);                                                     /* Initializes the global (static, technically) hgl_rita context. */
static inline void hgl_rita_final(void);                                                    /* Releases all resources held by the global hgl_rita context. Despawns all threads, destroys all queues, etc. */
static inline void hgl_rita_bind_buffer(HglRitaBuffer buffer, void *item);                  /* binds an item to the specified target in the current context. */
static inline void hgl_rita_bind_texture(HglRitaTexUnit unit, HglRitaTexture *tex);         /* binds a texture to the specified texture unit in the current context. */
static inline void hgl_rita_bind_vert_shader(HglRitaVertShaderFunc vert);                   /* binds the specified vertex shader in the current context. A value of NULL uses default vertex processing */
static inline void hgl_rita_bind_frag_shader(HglRitaFragShaderFunc frag);                   /* binds the specified fragment shader in the current context. A value of NULL uses default fragment processing */
static inline void hgl_rita_enable(uint32_t opts);                                          /* Enables the specified options in the current context (options may be bitwise OR:ed together. See HglRitaOpt.). */
static inline void hgl_rita_disable(uint32_t opts);                                         /* Disables the specified options in the current context (options may be bitwise OR:ed together. See HglRitaOpt.). */
static inline void hgl_rita_use_frontface_winding_order(HglRitaWindingOrder winding_order); /* Use the specified winding order to determine which triangle faces are front-facing in the current context. */
static inline void hgl_rita_use_clear_color(HglRitaColor color);                            /* Use the specified color when clearing the framebuffer color attachment in the current context. */
static inline void hgl_rita_use_texture_filter(HglRitaTextureFilter filter);                /* Use the specified texture filtering algorithm for texture sampling functions in the current context. */
static inline void hgl_rita_use_texture_wrapping(HglRitaTextureWrapping wrap_mode);         /* Use the specified texture wrapping for texture sampling functions in the current context. */
static inline void hgl_rita_use_vertex_buffer_mode(HglRitaVertexBufferMode mode);           /* Use the specified vertex buffer mode for draw calls (hgl_rita_draw) in the current context*/
static inline void hgl_rita_use_model_matrix(Mat4 m);                                       /* Use the specified model matrix in the current context*/
static inline void hgl_rita_use_view_matrix(Mat4 m);                                        /* Use the specified view matrix in the current context */
static inline void hgl_rita_use_proj_matrix(Mat4 m);                                        /* Use the specified projection matrix in the current context */
static inline void hgl_rita_use_camera_view(Vec3 pos, Vec3 tgt, Vec3 up);                   /* Construct a `look at` view matrix and use it (tform.view). Will populate the respective `tform.camera` attributes. */
static inline void hgl_rita_use_perspective_proj(float fov, float aspect, 
                                                 float znear, float zfar);                  /* Construct a perspective projection matrix and use it (tform.proj). Will populate the respective `tform.camera` attributes. */
static inline void hgl_rita_use_orthographic_proj(float left, float right, 
                                                  float bottom, float top,
                                                  float near, float far);                   /* Construct an orthographic projection matrix and use it (tform.proj). Will populate the respective `tform.camera` attributes. */
static inline void hgl_rita_use_viewport(int width, int height);                            /* Use the specified viewport dimensions in the current context. These values determine the transformation from NDC space to pixel coordinates. */

/* Drawing */
static inline void hgl_rita_clear(uint32_t attachments);                                    /* Clears the specified attachments of the currently bound framebuffer(attachments may be bitwise OR:ed together. See HglRitaFramebufferAttachment). */
static inline void hgl_rita_finish(void);                                                   /* Waits until all asynchronous operations (hgl_rita_draw, hgl_rita_blit) have finished. */
static inline void hgl_rita_draw_text(int pos_x, int pos_y,
                                      float scale,
                                      HglRitaColor color,
                                      const char *fmt, ...);                                /* Draws text at the given screen-space position. */
static inline void hgl_rita_draw(HglRitaPrimitiveMode primitive_mode);                      /* Draws the contents of the current bound vertex buffer using the selected primitive mode. This is an asynchronous operation. */
static inline void hgl_rita_blit(int x, int y, int w, int h,
                                 HglRitaTexture *src,
                                 HglRitaBlendMethod blend_method,
                                 HglRitaBlitFBMask mask,
                                 HglRitaBlitFBSampler sampling_method,
                                 HglRitaFragShaderFunc shader);                             /* Blits `src` onto the framebuffer color attachment at the specified region. This is an asynchronous operation. */

/* HglRitaTexture: standalone functions */
static inline HglRitaTexture hgl_rita_texture_make(int width, int height,
                                                   HglRitaPixelFormat format);              /* Allocates a new texture. Should be free'd using `hgl_rita_texture_destroy()` */
static inline void hgl_rita_texture_destroy(HglRitaTexture *tex);                           /* Destroys texture `tex`. The underlying memory buffer is freed. */
static inline HglRitaTexture hgl_rita_texture_get_subtexture(HglRitaTexture tex,
                                                             int x, int y,
                                                             int width, int height);        /* Creates a subtexture of `tex` at the given region. Must not be freed.*/
static inline void hgl_rita_texture_flip_vertically(HglRitaTexture *tex);                   /* Vertically flips the texture `tex`. */
static inline void hgl_rita_texture_blit(HglRitaTexture dst,
                                         HglRitaTexture src,
                                         HglRitaBlendMethod blend_method,
                                         bool flip_vertical);                               /* Blits `src` onto `dest` using the specified blend method. */

/* HglRitaVertex: standalone functions */
static inline bool hgl_rita_vertex_eq(HglRitaVertex v0, HglRitaVertex v1);                  /* Returns true if `v0` and `v1` are equal */

/* HglRitaColor: standalone functions */
static inline HglRitaColor hgl_rita_color_blend(HglRitaColor c0,
                                                HglRitaColor c1,
                                                HglRitaBlendMethod method);                 /* Blends two colors, `c0` and `c1`, using the given blend method. */
static inline HglRitaColor hgl_rita_color_lerp(HglRitaColor c0, HglRitaColor c1, float t);  /* Calculates the linear interpolation at t \in [0, 1] of the colors `c0` and `c1`. */
static inline HglRitaColor hgl_rita_color_add(HglRitaColor c0, HglRitaColor c1);            /* Calculates the color sum `c0` + `c1`. */
static inline HglRitaColor hgl_rita_color_sub(HglRitaColor c0, HglRitaColor c1);            /* Calculates the color difference `c0` - `c1`. */
static inline HglRitaColor hgl_rita_color_mul(HglRitaColor c0, HglRitaColor c1);            /* Calculates the color Hadamard product `c0` * `c1`. */
static inline HglRitaColor hgl_rita_color_mul_scalar(HglRitaColor c0, float s);             /* Calculates the color Hadamard product `c0` * `c1`. */
static inline bool hgl_rita_color_eq(HglRitaColor c0, HglRitaColor c1);                     /* Returns true if `c0` and `c1` are equal */
static inline float hgl_rita_color_luminance(HglRitaColor c0);                              /* Calculates the luminance of `c0` normalized to [0, 1]. */
static inline Vec4 hgl_rita_color_as_vector(HglRitaColor c0);                               /* Returns a vector where the x, y, z, and w components are the color channels r, g, b, and a normalized to [0, 1]. */
static inline HglRitaColor hgl_rita_color_from_vector(Vec4 v);                              /* Does the inverse of `hgl_rita_color_as_vector()` */

/* HglRitaAABB: standalone functions */
static inline HglRitaAABB hgl_rita_aabb_make(int x, int y, int w, int h);                   /* Creates n 2D Axis-aligned bounding box (AABB) with the given dimensions. */
static inline HglRitaAABB hgl_rita_aabb_from_line(HglRitaLine line);                        /* Creates the smallest AABB containing `line` */
static inline HglRitaAABB hgl_rita_aabb_from_tri(HglRitaTriangle tri);                      /* Creates the smallest AABB containing `tri` */
static inline HglRitaAABB hgl_rita_aabb_clip(HglRitaAABB aabb, int min_x,
                                             int min_y, int max_x, int max_y);              /* Clips `aabb` such that it is contained inside [x_min, x_max] on the x-axis and [y_min, y_max] on the y-axis. */
static inline HglRitaAABB hgl_rita_aabb_intersection(HglRitaAABB a, HglRitaAABB b);         /* Returns the intersection of two bounding boxes `a` and `b` */
static inline bool hgl_rita_aabb_intersects(HglRitaAABB a, HglRitaAABB b);                  /* Returns true if the two bounding boxes `a` and `b` intersect. */

/* texture sampling */
static inline HglRitaColor hgl_rita_sample(HglRitaTexture *tex, int x, int y);              /* Samples `tex` at the texel position (`x`, `y`).*/
static inline HglRitaColor hgl_rita_sample_uv(HglRitaTexture *tex, Vec2 uv);                /* Samples `tex` at the 2D texture coordinate `uv` */
static inline HglRitaColor hgl_rita_sample_rectilinear(HglRitaTexture *tex, Vec3 dir);      /* Samples `tex` using rectilinear projection at the 3D view direction `dir` */
static inline HglRitaColor hgl_rita_sample_cubemap(HglRitaTexture *tex, Vec3 dir);          /* Samples `tex` using cubemap projection at the 3D view direction `dir` */
static inline HglRitaColor hgl_rita_sample_unit(HglRitaTexUnit unit, int x, int y);         /* Samples the texture bound to texture unit `unit` at the texel position (`x`, `y`).*/
static inline HglRitaColor hgl_rita_sample_unit_uv(HglRitaTexUnit unit, Vec2 uv);           /* Samples the texture bound to texture unit `unit` at the 2D texture coordinate `uv` */
static inline HglRitaColor hgl_rita_sample_unit_rectilinear(HglRitaTexUnit unit, Vec3 dir); /* Samples the texture bound to texture unit `unit` using rectilinear projection at the 3D view direction `dir` */
static inline HglRitaColor hgl_rita_sample_unit_cubemap(HglRitaTexUnit unit, Vec3 dir);     /* Samples the texture bound to texture unit `unit` using cubemap projection at the 3D view direction `dir` */

/* internal functions */
static inline void *hgl_rita_tile_thread_internal_(void *arg);                              /* This function contains the main work-loop of each spawned tile thread. */
static inline void hgl_rita_dispatch_point_internal_(HglRitaFragment f0);                   /* Dispatches a point/pixel primitive to the thread of the tile containing it */
static inline void hgl_rita_dispatch_line_internal_(HglRitaFragment f0,
                                                    HglRitaFragment f1);                    /* Dispatches a line primitive to the threads of the tiles intersecting its AABB */
static inline void hgl_rita_dispatch_tri_internal_(HglRitaFragment f0,
                                                   HglRitaFragment f1,
                                                   HglRitaFragment f2);                     /* Dispatches a triangle primitive to the threads of the tiles intersecting its AABB */
static inline HglRitaFragment hgl_rita_process_vertex_internal_(const HglRitaVertex *in);   /* Processes a single vertex into a fragment and returns it. This function contains the VERTEX SHADER step! */
static inline void hgl_rita_process_fragment_internal_(HglRitaFragment *in);                /* Processes a single fragment. If the fragment is accepted, it is drawn to the frame buffer. This function contains the FRAGMENT SHADER step! */
static inline HglRitaFragment hgl_rita_frag_lerp_internal_(int x, int y,
                                                           HglRitaFragment f0,
                                                           HglRitaFragment f1,
                                                           float t);                        /* Linearly interpolates between two fragments `f0` and `f1` */
static inline HglRitaFragment hgl_rita_frag_berp_internal_(HglRitaFragment f0,
                                                           HglRitaFragment f1,
                                                           HglRitaFragment f2,
                                                           float u, float v,
                                                           int x, int y);                   /* Interpolates between three fragments `f0`, `f1`, and `f2`, given the barycentric coordinate (`u`, `v`, 1 - `u` - `v`) */
static inline float hgl_rita_det_internal_(int f0_x, int f0_y,
                                           int f1_x, int f1_y,
                                           int f2_x, int f2_y);                             /* Cheeky determinant which isn't really a determinant. Something to do with a '2D cross product'. */
static inline int hgl_rita_next_vbuf_index_internal_(void);                                 /* Fetches the next vertex in the vertex buffer given the current vertex buffer mode (HGL_RITA_ARRAY or HGL_RITA_INDEXED) */

#endif /* HGL_RITA_H */

#ifdef HGL_RITA_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sysinfo.h>

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static HglRitaContext hgl_rita_ctx__;

/*--- Public functions ------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/
/*--- General functions -----------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

static inline void hgl_rita_init()
{
    nice(-10);

    /* setup shaders */
    hgl_rita_ctx__.shaders.vert = NULL;
    hgl_rita_ctx__.shaders.frag = NULL;

    /* setup vertex buffer */
    hgl_rita_ctx__.vertices.mode = HGL_RITA_ARRAY;
    hgl_rita_ctx__.vertices.vbuf = NULL;
    hgl_rita_ctx__.vertices.ibuf = NULL;
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
    hgl_rita_ctx__.vertices.fbuf = (HglRitaFragmentBuffer){0};
    hgl_rita_buf_reserve(&hgl_rita_ctx__.vertices.fbuf, 4096);
#endif

    /* setup default opts */
    hgl_rita_ctx__.opts.frontface_winding                       = HGL_RITA_CCW;
    hgl_rita_ctx__.opts.clear_color                             = HGL_RITA_MORTEL_BLACK;
    hgl_rita_ctx__.opts.texture_filter                          = HGL_RITA_NEAREST;
    hgl_rita_ctx__.opts.texture_wrapping                        = HGL_RITA_NO_WRAPPING;
    hgl_rita_ctx__.opts.backface_culling_enabled                = false;
    hgl_rita_ctx__.opts.depth_test_enabled                      = false;
    hgl_rita_ctx__.opts.order_dependent_alpha_blending_enabled  = false;
    hgl_rita_ctx__.opts.z_clipping_enabled                      = false;
    hgl_rita_ctx__.opts.depth_buffer_writing_enabled            = true;
    hgl_rita_ctx__.opts.draw_wire_frames                        = false;

    /* setup default transforms */
    hgl_rita_ctx__.tform.model           = mat4_make_identity();
    hgl_rita_ctx__.tform.view            = mat4_make_identity();
    hgl_rita_ctx__.tform.proj            = mat4_make_identity();
    hgl_rita_ctx__.tform.viewport        = mat4_make_identity();
    hgl_rita_ctx__.tform.normals         = mat3_make_identity();
    hgl_rita_ctx__.tform.iview           = mat3_make_identity();
    hgl_rita_ctx__.tform.camera.position = vec3_make(0,0,0);
    hgl_rita_ctx__.tform.camera.target   = vec3_make(0,0,0);
    hgl_rita_ctx__.tform.camera.up       = vec3_make(0,1,0);
    hgl_rita_ctx__.tform.camera.fov      = 0.0f;
    hgl_rita_ctx__.tform.camera.aspect   = 1.0f;
    hgl_rita_ctx__.tform.camera.znear    = 0.0f;
    hgl_rita_ctx__.tform.camera.zfar     = 1.0f;

    /* setup texture units */
    for (int i = 0; i < HGL_RITA_N_TEXTURE_UNITS - 2; i++) {
        hgl_rita_ctx__.tex_unit[0] = NULL;
    }

    /* Initialize renderer */
    hgl_rita_ctx__.renderer.n_tiles = 0;
    hgl_rita_ctx__.renderer.n_tile_cols = 0;
    hgl_rita_ctx__.renderer.n_tile_rows = 0;
    hgl_rita_ctx__.renderer.n_procs = get_nprocs();
}

static inline void hgl_rita_final(void)
{
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
    hgl_rita_buf_destroy(&hgl_rita_ctx__.vertices.fbuf);
#endif

    for (int i = 0; i < hgl_rita_ctx__.renderer.n_tiles; i++) {
        HglRitaTileOp op = { .kind = HGL_RITA_OP_TERMINATE };
        hgl_rita_queue_push(&hgl_rita_ctx__.renderer.tile[i].op_queue, op);
        pthread_join(hgl_rita_ctx__.renderer.tile[i].thread, NULL);
        hgl_rita_queue_destroy(&hgl_rita_ctx__.renderer.tile[i].op_queue);
    }
    hgl_rita_ctx__.renderer.n_tiles = 0;
}

static inline void hgl_rita_bind_buffer(HglRitaBuffer buffer, void *item)
{
    switch (buffer) {
        case HGL_RITA_VERTEX_BUFFER: {
            hgl_rita_ctx__.vertices.vbuf = (HglRitaVertexBuffer *) item;
        } break;

        case HGL_RITA_INDEX_BUFFER: {
            hgl_rita_ctx__.vertices.ibuf = (HglRitaIndexBuffer *) item;
        } break;
    }
}

static inline void hgl_rita_bind_texture(HglRitaTexUnit unit, HglRitaTexture *tex)
{
    hgl_rita_finish();
    if (unit == HGL_RITA_TEX_FRAME_BUFFER) {
        assert(tex->format == HGL_RITA_RGBA8);

        int w = tex->width;
        int h = tex->height;
        int cols = (w - 1) / HGL_RITA_TILE_SIZE_X + 1;
        int rows = (h - 1) / HGL_RITA_TILE_SIZE_Y + 1;
        int n_active_tiles = hgl_rita_ctx__.renderer.n_tiles;
        int n_needed_tiles = cols * rows;

        /* Needs more tiles than allowed? */
        if (n_needed_tiles > HGL_RITA_MAX_N_TILES) {
            fprintf(stderr, "framebuffer texture too large. Consider increasing HGL_RITA_MAX_N_TILES\n");
            fprintf(stderr, "or the tile dimensions HGL_RITA_TILE_SIZE_X/HGL_RITA_TILE_SIZE_Y.");
            exit(1);
        }

        /* Spawn more tile workers if necessary */
        for (int i = n_active_tiles; i < n_needed_tiles; i++) {
            HglRitaTile *tile = &hgl_rita_ctx__.renderer.tile[i];
            hgl_rita_queue_init(&tile->op_queue);
            tile->aabb = hgl_rita_aabb_make((i%cols)*HGL_RITA_TILE_SIZE_X,
                                            (i/cols)*HGL_RITA_TILE_SIZE_Y,
                                            HGL_RITA_TILE_SIZE_X,
                                            HGL_RITA_TILE_SIZE_Y);
            tile->aabb = hgl_rita_aabb_clip(tile->aabb, 0, 0, w, h);
            pthread_create(&tile->thread, NULL, hgl_rita_tile_thread_internal_, (void *)tile);
        }
        hgl_rita_ctx__.renderer.n_tiles = n_needed_tiles;
        hgl_rita_ctx__.renderer.n_tile_cols = cols;
        hgl_rita_ctx__.renderer.n_tile_rows = rows;
    } else if (unit == HGL_RITA_TEX_DEPTH_BUFFER) {
        assert(tex->format == HGL_RITA_R32);
    }

    hgl_rita_ctx__.tex_unit[unit] = tex;
}

static inline void hgl_rita_bind_vert_shader(HglRitaVertShaderFunc vert)
{
    hgl_rita_ctx__.shaders.vert = vert;
}

static inline void hgl_rita_bind_frag_shader(HglRitaFragShaderFunc frag)
{
    hgl_rita_ctx__.shaders.frag = frag;
}

static inline void hgl_rita_enable(uint32_t opts)
{
    if (opts & HGL_RITA_BACKFACE_CULLING) {
        hgl_rita_ctx__.opts.backface_culling_enabled = true;
    }
    if (opts & HGL_RITA_DEPTH_TESTING) {
        hgl_rita_ctx__.opts.depth_test_enabled = true;
    }
    if (opts & HGL_RITA_ORDER_DEPENDENT_ALPHA_BLEND) {
        hgl_rita_ctx__.opts.order_dependent_alpha_blending_enabled = true;
    }
    if (opts & HGL_RITA_Z_CLIPPING) {
        hgl_rita_ctx__.opts.z_clipping_enabled = true;
    }
    if (opts & HGL_RITA_DEPTH_BUFFER_WRITING) {
        hgl_rita_ctx__.opts.depth_buffer_writing_enabled = true;
    }
    if (opts & HGL_RITA_WIRE_FRAMES) {
        hgl_rita_ctx__.opts.draw_wire_frames = true;
    }
}

static inline void hgl_rita_disable(uint32_t opts)
{
    if (opts & HGL_RITA_BACKFACE_CULLING) {
        hgl_rita_ctx__.opts.backface_culling_enabled = false;
    }
    if (opts & HGL_RITA_DEPTH_TESTING) {
        hgl_rita_ctx__.opts.depth_test_enabled = false;
    }
    if (opts & HGL_RITA_ORDER_DEPENDENT_ALPHA_BLEND) {
        hgl_rita_ctx__.opts.order_dependent_alpha_blending_enabled = false;
    }
    if (opts & HGL_RITA_Z_CLIPPING) {
        hgl_rita_ctx__.opts.z_clipping_enabled = false;
    }
    if (opts & HGL_RITA_DEPTH_BUFFER_WRITING) {
        hgl_rita_ctx__.opts.depth_buffer_writing_enabled = false;
    }
    if (opts & HGL_RITA_WIRE_FRAMES) {
        hgl_rita_ctx__.opts.draw_wire_frames = false;
    }
}

static inline void hgl_rita_use_frontface_winding_order(HglRitaWindingOrder winding_order)
{
    hgl_rita_ctx__.opts.frontface_winding = winding_order;
}

static inline void hgl_rita_use_clear_color(HglRitaColor color)
{
    hgl_rita_ctx__.opts.clear_color = color;
}

static inline void hgl_rita_use_texture_filter(HglRitaTextureFilter filter)
{
    hgl_rita_ctx__.opts.texture_filter = filter;
}

static inline void hgl_rita_use_texture_wrapping(HglRitaTextureWrapping wrap_mode)
{
    hgl_rita_ctx__.opts.texture_wrapping = wrap_mode;
}

static inline void hgl_rita_use_vertex_buffer_mode(HglRitaVertexBufferMode mode)
{
    hgl_rita_ctx__.vertices.mode = mode;
}

static inline void hgl_rita_use_model_matrix(Mat4 m)
{
    hgl_rita_ctx__.tform.model = m;
    Mat3 m_normals = mat3_make_from_mat4(m);
    float c0_len = vec3_len(m_normals.c0);
    float c1_len = vec3_len(m_normals.c1);
    float c2_len = vec3_len(m_normals.c2);
    m_normals.c0 = vec3_mul_scalar(m_normals.c0, 1.0f / c0_len);
    m_normals.c1 = vec3_mul_scalar(m_normals.c1, 1.0f / c1_len);
    m_normals.c2 = vec3_mul_scalar(m_normals.c2, 1.0f / c2_len);
    hgl_rita_ctx__.tform.normals = m_normals;
}

static inline void hgl_rita_use_view_matrix(Mat4 m)
{
    hgl_rita_ctx__.tform.view = m;
    hgl_rita_ctx__.tform.iview = mat3_transpose(mat3_make_from_mat4(m));
}

static inline void hgl_rita_use_proj_matrix(Mat4 m)
{
    hgl_rita_ctx__.tform.proj = m;
}

static inline void hgl_rita_use_camera_view(Vec3 pos, Vec3 tgt, Vec3 up)
{
    Mat4 m = mat4_look_at(pos, tgt, up);
    hgl_rita_use_view_matrix(m);
    hgl_rita_ctx__.tform.camera.position = pos;
    hgl_rita_ctx__.tform.camera.target   = tgt;
    hgl_rita_ctx__.tform.camera.up       = up;
}

static inline void hgl_rita_use_perspective_proj(float fov, float aspect, 
                                                 float znear, float zfar)
{
    Mat4 m = mat4_make_perspective(fov, aspect, znear, zfar);
    hgl_rita_use_proj_matrix(m);
    hgl_rita_ctx__.tform.camera.fov    = fov;
    hgl_rita_ctx__.tform.camera.aspect = aspect;
    hgl_rita_ctx__.tform.camera.znear  = znear;
    hgl_rita_ctx__.tform.camera.zfar   = zfar;
}

static inline void hgl_rita_use_orthographic_proj(float left, float right, 
                                                  float bottom, float top,
                                                  float near, float far)
{
    Mat4 m = mat4_make_ortho(left, right, bottom, top, near, far);
    hgl_rita_ctx__.tform.proj          = m;
    hgl_rita_ctx__.tform.camera.fov    = 0.0f;
    hgl_rita_ctx__.tform.camera.aspect = 1.0f;
    hgl_rita_ctx__.tform.camera.znear  = near;
    hgl_rita_ctx__.tform.camera.zfar   = far;
}


static inline void hgl_rita_use_viewport(int width, int height)
{
    Mat4 m = mat4_make_translation(vec3_make((float)width/2.0f, (float)height/2.0f, 0.0f));
    m = mat4_scale(m, vec3_make(width/2.0f, -height/2.0f, 1.0f));
    hgl_rita_ctx__.tform.viewport = m;
}


/*---------------------------------------------------------------------------------------*/
/*--- Drawing ---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

static inline void hgl_rita_clear(uint32_t attachments)
{
    int w, h;

    if (attachments & HGL_RITA_COLOR) {
        w = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->width;
        h = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->height;
        for (int i = 0; i < w*h; i++) {
            hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->data.rgba8[i] = hgl_rita_ctx__.opts.clear_color;
        }
    }

    if (attachments & HGL_RITA_DEPTH) {
        w = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER]->width;
        h = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER]->height;
        for (int i = 0; i < w*h; i++) {
            hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER]->data.r32[i] = 1.0f;
        }
    }
}

static inline void hgl_rita_finish(void)
{
    for (int i = 0; i < hgl_rita_ctx__.renderer.n_tiles; i++) {
        hgl_rita_queue_wait_until_idle(&hgl_rita_ctx__.renderer.tile[i].op_queue, 1);
    }
}

static inline void hgl_rita_draw_text(int pos_x, int pos_y, float scale, HglRitaColor color, const char *fmt, ...)
{

    /*
     * Note: each glyph has been compacted into a single line with the bitmap
     *       written as a series of decimal numbers. Written using binary
     *       numbers, an entry really looks like this (highlight the 1's
     *       in your text editor for a clearer view of what's going on):
     *
     *    ['A'] = {
     *        .bitmap = {
     *            0b00111100,
     *            0b01100110,
     *            0b11111111,
     *            0b11000011,
     *            0b11000011,
     *            0b11000011,
     *        }, 8, 0
     *    },
     */
    static const HglRitaGlyph HGL_RITA_FONT[127] = {
        [0]   = { .bitmap = { 0x55,0xAA,0x55,0xAA,0x55,0xAA,}, 6, 0}, // dummy/"missing" glyph
        [' '] = { .bitmap = {0,0,0,0,0,0}, 5, 0},
        ['%'] = { .bitmap = {0,17,18,4,9,17}, 5, 0},
        ['+'] = { .bitmap = {0,4,4,31,4,4}, 5, 0},
        ['-'] = { .bitmap = {0,0,0,31,0,0}, 5, 0},
        ['_'] = { .bitmap = {0,0,0,0,0,127}, 7, 0},
        ['*'] = { .bitmap = {0,0,5,2,5,0}, 3, 0},
        ['/'] = { .bitmap = {0,1,2,4,8,16}, 5, 0},
        ['.'] = { .bitmap = {0,0,0,0,0,1}, 1, 0},
        [':'] = { .bitmap = {0,0,1,0,1,0}, 1, 0},
        [';'] = { .bitmap = {0,0,1,0,1,2}, 2, 0},
        ['<'] = { .bitmap = {0,1,2,4,2,1}, 3, 0},
        ['='] = { .bitmap = {0,0,31,0,31,0}, 5, 0},
        ['>'] = { .bitmap = {0,4,2,1,2,4}, 3, 0},
        ['!'] = { .bitmap = {1,1,1,1,0,1}, 1, 0},
        ['?'] = { .bitmap = {6,9,1,2,0,2}, 4, 0},
        ['('] = { .bitmap = {1,2,2,2,2,1}, 2, 0},
        [')'] = { .bitmap = {2,1,1,1,1,2}, 2, 0},
        ['['] = { .bitmap = {3,2,2,2,2,3}, 2, 0},
        [']'] = { .bitmap = {3,1,1,1,1,3}, 2, 0},
        ['0'] = { .bitmap = {14,17,23,25,17,14}, 5, 0},
        ['1'] = { .bitmap = {2,6,2,2,2,7}, 3, 0},
        ['2'] = { .bitmap = {14,17,2,4,8,31}, 5, 0},
        ['3'] = { .bitmap = {14,17,6,1,17,14}, 5, 0},
        ['4'] = { .bitmap = {17,17,15,1,1,1}, 5, 0},
        ['5'] = { .bitmap = {31,16,30,1,17,14}, 5, 0},
        ['6'] = { .bitmap = {14,16,30,17,17,14}, 5, 0},
        ['7'] = { .bitmap = {31,1,2,4,4,4}, 5, 0},
        ['8'] = { .bitmap = {14,17,14,17,17,14}, 5, 0},
        ['9'] = { .bitmap = {14,17,17,15,1,14}, 5, 0},
        ['A'] = { .bitmap = {60,102,255,195,195,195}, 8, 0},
        ['B'] = { .bitmap = {254,195,254,195,195,254}, 8, 0},
        ['C'] = { .bitmap = {127,192,192,192,192,127}, 8, 0},
        ['D'] = { .bitmap = {252,195,195,195,195,252}, 8, 0},
        ['E'] = { .bitmap = {255,192,252,192,192,255}, 8, 0},
        ['F'] = { .bitmap = {255,192,252,192,192,192}, 8, 0},
        ['G'] = { .bitmap = {126,192,198,195,195,126}, 8, 0},
        ['H'] = { .bitmap = {195,195,255,195,195,195}, 8, 0},
        ['I'] = { .bitmap = {15,6,6,6,6,15}, 4, 0},
        ['J'] = { .bitmap = {15,6,6,6,102,60}, 7, 0},
        ['K'] = { .bitmap = {102,108,120,108,102,99}, 7, 0},
        ['L'] = { .bitmap = {96,96,96,96,96,127}, 7, 0},
        ['M'] = { .bitmap = {195,231,219,195,195,195}, 8, 0},
        ['N'] = { .bitmap = {195,227,211,203,199,195}, 8, 0},
        ['O'] = { .bitmap = {60,195,195,195,195,60}, 8, 0},
        ['P'] = { .bitmap = {252,195,252,192,192,192}, 8, 0},
        ['Q'] = { .bitmap = {60,195,195,203,198,61}, 8, 0},
        ['R'] = { .bitmap = {252,195,252,216,204,198}, 8, 0},
        ['S'] = { .bitmap = {126,195,120,6,195,126}, 8, 0},
        ['T'] = { .bitmap = {255,24,24,24,24,24}, 8, 0},
        ['U'] = { .bitmap = {195,195,195,195,195,60}, 8, 0},
        ['V'] = { .bitmap = {99,99,99,54,28,8}, 7, 0},
        ['W'] = { .bitmap = {195,195,195,219,231,195}, 8, 0},
        ['X'] = { .bitmap = {195,102,60,60,102,195}, 8, 0},
        ['Y'] = { .bitmap = {195,102,60,24,24,24}, 8, 0},
        ['Z'] = { .bitmap = {127,6,12,24,48,127}, 7, 0},
        ['a'] = { .bitmap = {0,15,17,17,19,13}, 5, 0},
        ['b'] = { .bitmap = {16,30,17,17,17,30}, 5, 0},
        ['c'] = { .bitmap = {0,7,8,8,8,7}, 4, 0},
        ['d'] = { .bitmap = {1,15,17,17,17,15}, 5, 0},
        ['e'] = { .bitmap = {0,15,17,31,16,15}, 5, 0},
        ['f'] = { .bitmap = {0,15,16,30,16,16}, 5, 0},
        ['g'] = { .bitmap = {15,17,17,15,1,14}, 5, 1},
        ['h'] = { .bitmap = {16,22,25,17,17,17}, 5, 0},
        ['i'] = { .bitmap = {0,1,0,1,1,1}, 1, 0},
        ['j'] = { .bitmap = {1,0,1,1,5,3}, 3, 1},
        ['k'] = { .bitmap = {16,18,20,28,18,17}, 5, 0},
        ['l'] = { .bitmap = {1,1,1,1,1,1}, 1, 0},
        ['m'] = { .bitmap = {0,26,21,21,21,21}, 5, 0},
        ['n'] = { .bitmap = {0,14,9,9,9,9}, 4, 0},
        ['o'] = { .bitmap = {0,6,9,9,9,6}, 4, 0},
        ['p'] = { .bitmap = {14,9,9,9,14,8}, 4, 1},
        ['q'] = { .bitmap = {7,9,9,9,7,1}, 4, 1},
        ['r'] = { .bitmap = {0,11,12,8,8,8}, 4, 0},
        ['s'] = { .bitmap = {0,7,8,6,1,14}, 4, 0},
        ['t'] = { .bitmap = {4,15,4,4,4,3}, 4, 0},
        ['u'] = { .bitmap = {0,9,9,9,9,7}, 4, 0},
        ['v'] = { .bitmap = {0,17,17,17,10,4}, 5, 0},
        ['w'] = { .bitmap = {0,17,17,17,21,10}, 5, 0},
        ['x'] = { .bitmap = {0,17,10,4,10,17}, 5, 0},
        ['y'] = { .bitmap = {9,9,9,7,1,14}, 4, 1},
        ['z'] = { .bitmap = {0,15,1,2,4,15}, 4, 0},
    };

    const int og_pos_x = pos_x;
    const float x_spacing = 1.0f;
    const float y_spacing = 1.0f;
    const int fb_w = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->width;
    const int fb_h = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->height;
    (void) fb_h;

    /* print formatted string into scratch buffer */
    va_list args;
    va_start(args, fmt);
    static char scratch[HGL_RITA_TEXT_BUFFER_MAX_SIZE];
    vsnprintf(scratch, HGL_RITA_TEXT_BUFFER_MAX_SIZE, fmt, args);
    va_end(args);
    char *formatted_str = scratch;

    /* make sure all parallel framebuffer ops are complete */
    hgl_rita_finish();

    while (true) {
        char c = *formatted_str++;

        switch (c) {
            case '\0': return;

            case '\n': {
                pos_x = og_pos_x;
                pos_y += (6.0f + y_spacing) * scale;
            } continue;

            default: {
                HglRitaGlyph glyph = HGL_RITA_FONT[(int)clamp(0, 127, c)];
                if(glyph.stride == 0) {
                    glyph = HGL_RITA_FONT[0]; // dummy glyph
                }

                int box_x_start = pos_x;
                int box_x_end   = box_x_start + scale * glyph.stride;
                int box_y_start = pos_y + scale * glyph.vertical_offset;;
                int box_y_end   = box_y_start + 6.0f*scale;

                for (int y = box_y_start; y < box_y_end; y++) {
                    if (y < 0 || y >= fb_h) {
                        continue;
                    }
                    for (int x = box_x_start; x < box_x_end; x++) {
                        if (x < 0 || x >= fb_w) {
                            continue;
                        }
                        float u = (float)(x - box_x_start) / (float) (box_x_end - box_x_start);
                        float v = (float)(y - box_y_start) / (float) (box_y_end - box_y_start);
                        int idx = y*fb_w + x;
                        int row = v * 6;
                        int col = u * glyph.stride;
                        if (((glyph.bitmap[row] >> ((glyph.stride - 1) - col)) & 1) != 0) {
                            hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->data.rgba8[idx] = color;
                        }
                    }
                }
                pos_x = box_x_end + scale * x_spacing;
            }
        }
    }
}

static inline void hgl_rita_draw(HglRitaPrimitiveMode primitive_mode)
{
    int i0;
    int i1;
    int i2;
    HglRitaFragment f0;
    HglRitaFragment f1;
    HglRitaFragment f2;
#ifndef HGL_RITA_PARALLEL_VERTEX_PROCESSING
    HglRitaVertex *v0;
    HglRitaVertex *v1;
    HglRitaVertex *v2;
#endif

    /* reset counter used to get next vertex */
    hgl_rita_ctx__.vertices.counter = 0;

    /* compute mvp matrix to (potentially) be used in vertex shader */
    Mat4 m = hgl_rita_ctx__.tform.model;
    Mat4 v = hgl_rita_ctx__.tform.view;
    Mat4 p = hgl_rita_ctx__.tform.proj;
    hgl_rita_ctx__.tform.mv = mat4_mul_mat4(v, m);
    hgl_rita_ctx__.tform.mvp = mat4_mul_mat4(p, mat4_mul_mat4(v, m));

#ifdef HGL_RITA_DEBUG
    HglRitaTexture *fb = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER];
    HglRitaTexture *db = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER];
    bool depth_ops_enabled = hgl_rita_ctx__.depth_test_enabled ||
                             hgl_rita_ctx__.depth_buffer_writing_enabled;
    if ((db == NULL) && depth_ops_enabled) {
        fprintf(stderr, "[hgl_rita] Warning: depth testing and/or depth buffer writing is enabled but no depth buffer is bound.\n");
    }
    if ((db != NULL) && (fb != NULL) && depth_ops_enabled) {
        if ((fb->width != db->width) || (fb->height != db->height)) {
            fprintf(stderr, "[hgl_rita] Warning: depth testing and/or depth buffer writing is enabled but the depth and frame \n");
            fprintf(stderr, "                    buffers have different dimensions.\n");
        }
    }
#endif

    /*
     * If HGL_RITA_PARALLEL_VERTEX_PROCESSING is definied, vertices are processed
     * up-front and ahead-of-time. Otherwise, vertices are processed just-in-time
     * as primitives are dispatched.
     */
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
    /*
     * make sure previous drawcall has finished so that we don't modify
     * the fragment buffer as it is being used
     */
    hgl_rita_finish();

    /* Dispatch chunks of the vertex buffer to be proceesed in parallel */
    hgl_rita_buf_reserve(&hgl_rita_ctx__.vertices.fbuf,
                         hgl_rita_ctx__.vertices.vbuf->length);
    int n_seg = min(hgl_rita_ctx__.renderer.n_procs - 1, hgl_rita_ctx__.renderer.n_tiles);
    int seg_sz = hgl_rita_ctx__.vertices.vbuf->length / n_seg;
    for (int i = 0; i < n_seg; i++) {
        HglRitaTileOp op = {
            .vbuf_segment = {
                .start_idx = i * seg_sz,
                .end_idx   = (i + 1) * seg_sz,
            },
            .kind = HGL_RITA_OP_PROCESS_VBUF_SEGMENT,
        };
        hgl_rita_queue_push(&hgl_rita_ctx__.renderer.tile[i].op_queue, op);
    }

    /* process remaining vertices in current thread */
    int rem_start = seg_sz * n_seg;
    int rem_end = hgl_rita_ctx__.vertices.vbuf->length;
    for (int i = rem_start; i < rem_end; i++) {
        const HglRitaVertex *v = &hgl_rita_ctx__.vertices.vbuf->arr[i];
        hgl_rita_ctx__.vertices.fbuf.arr[i] = hgl_rita_process_vertex_internal_(v);
    }

    /* rendezvous with the parallel workers */
    for (int i = 0; i < n_seg; i++) {
        hgl_rita_queue_wait_until_idle(&hgl_rita_ctx__.renderer.tile[i].op_queue, 1);
    }
#endif

    switch (primitive_mode) {
        case HGL_RITA_POINTS: {
            for (;;) {
                i0 = hgl_rita_next_vbuf_index_internal_(); if (i0 == -1) { break; }
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
                f0 = hgl_rita_ctx__.vertices.fbuf.arr[i0];
#else
                v0 = &hgl_rita_ctx__.vertices.vbuf->arr[i0];
                f0 = hgl_rita_process_vertex_internal_(v0);
#endif
                hgl_rita_dispatch_point_internal_(f0);
            }
        } break;

        case HGL_RITA_LINES: {
            for (;;) {
                i0 = hgl_rita_next_vbuf_index_internal_();
                i1 = hgl_rita_next_vbuf_index_internal_();
                if ((i0 == -1) || (i1 == -1)) {
                    break;
                }

#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
                f0 = hgl_rita_ctx__.vertices.fbuf.arr[i0];
                f1 = hgl_rita_ctx__.vertices.fbuf.arr[i1];
#else
                v0 = &hgl_rita_ctx__.vertices.vbuf->arr[i0];
                v1 = &hgl_rita_ctx__.vertices.vbuf->arr[i1];
                f0 = hgl_rita_process_vertex_internal_(v0);
                f1 = hgl_rita_process_vertex_internal_(v1);
#endif
                hgl_rita_dispatch_line_internal_(f0, f1);
            }
        } break;

        case HGL_RITA_LINE_STRIP: {
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
            i0 = hgl_rita_next_vbuf_index_internal_(); if (i0 == -1) { break; }
            f0 = hgl_rita_ctx__.vertices.fbuf.arr[i0];

            for (;;) {
                i1 = hgl_rita_next_vbuf_index_internal_(); if (i1 == -1) { break; }
                f1 = hgl_rita_ctx__.vertices.fbuf.arr[i1];
                hgl_rita_dispatch_line_internal_(f0, f1);
                f0 = f1;
            }
#else
            i0 = hgl_rita_next_vbuf_index_internal_(); if (i0 == -1) { break; }
            v0 = &hgl_rita_ctx__.vertices.vbuf->arr[i0];
            f0 = hgl_rita_process_vertex_internal_(v0);

            for (;;) {
                i1 = hgl_rita_next_vbuf_index_internal_(); if (i1 == -1) { break; }
                v1 = &hgl_rita_ctx__.vertices.vbuf->arr[i1];
                f1 = hgl_rita_process_vertex_internal_(v1);
                hgl_rita_dispatch_line_internal_(f0, f1);

                f0 = f1;
                v0 = v1;
            }
#endif
        } break;

        case HGL_RITA_TRIANGLES: {
            for (;;) {
                i0 = hgl_rita_next_vbuf_index_internal_();
                i1 = hgl_rita_next_vbuf_index_internal_();
                i2 = hgl_rita_next_vbuf_index_internal_();
                if ((i0 == -1) || (i1 == -1) || (i2 == -1)) {
                    break;
                }

#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
                f0 = hgl_rita_ctx__.vertices.fbuf.arr[i0];
                f1 = hgl_rita_ctx__.vertices.fbuf.arr[i1];
                f2 = hgl_rita_ctx__.vertices.fbuf.arr[i2];
#else
                v0 = &hgl_rita_ctx__.vertices.vbuf->arr[i0];
                v1 = &hgl_rita_ctx__.vertices.vbuf->arr[i1];
                v2 = &hgl_rita_ctx__.vertices.vbuf->arr[i2];
                f0 = hgl_rita_process_vertex_internal_(v0);
                f1 = hgl_rita_process_vertex_internal_(v1);
                f2 = hgl_rita_process_vertex_internal_(v2);
#endif
                hgl_rita_dispatch_tri_internal_(f0, f1, f2);
            }
        } break;

        case HGL_RITA_TRIANGLE_STRIP: {
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
            i0 = hgl_rita_next_vbuf_index_internal_();
            i1 = hgl_rita_next_vbuf_index_internal_();
            if ((i0 == -1) || (i1 == -1)) {
                break;
            }
            f0 = hgl_rita_ctx__.vertices.fbuf.arr[i0];
            f1 = hgl_rita_ctx__.vertices.fbuf.arr[i1];

            for (bool even = true;; even = !even) {
                i2 = hgl_rita_next_vbuf_index_internal_(); if (i2 == -1) { break; }
                f2 = hgl_rita_ctx__.vertices.fbuf.arr[i2];
                if (even) {
                    hgl_rita_dispatch_tri_internal_(f0, f1, f2);
                } else {
                    hgl_rita_dispatch_tri_internal_(f0, f2, f1);
                }
                f0 = f1;
                f1 = f2;
            }
#else
            i0 = hgl_rita_next_vbuf_index_internal_();
            i1 = hgl_rita_next_vbuf_index_internal_();
            if ((i0 == -1) || (i1 == -1)) {
                break;
            }
            v0 = &hgl_rita_ctx__.vertices.vbuf->arr[i0];
            v1 = &hgl_rita_ctx__.vertices.vbuf->arr[i1];
            f0 = hgl_rita_process_vertex_internal_(v0);
            f1 = hgl_rita_process_vertex_internal_(v1);

            for (bool even = true;; even = !even) {
                i2 = hgl_rita_next_vbuf_index_internal_(); if (i2 == -1) { break; }
                v2 = &hgl_rita_ctx__.vertices.vbuf->arr[i2];
                f2 = hgl_rita_process_vertex_internal_(v2);
                if (even) {
                    hgl_rita_dispatch_tri_internal_(f0, f1, f2);
                } else {
                    hgl_rita_dispatch_tri_internal_(f0, f2, f1);
                }
                f0 = f1;
                v0 = v1;
                f1 = f2;
                v1 = v2;
            }
#endif
        } break;

        case HGL_RITA_TRIANGLE_FAN: {
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
            i0 = hgl_rita_next_vbuf_index_internal_();
            i1 = hgl_rita_next_vbuf_index_internal_();
            if ((i0 == -1) || (i1 == -1)) {
                break;
            }
            f0 = hgl_rita_ctx__.vertices.fbuf.arr[i0];
            f1 = hgl_rita_ctx__.vertices.fbuf.arr[i1];
            for (;;) {
                i2 = hgl_rita_next_vbuf_index_internal_(); if (i2 == -1) { break; }
                f2 = hgl_rita_ctx__.vertices.fbuf.arr[i2];
                hgl_rita_dispatch_tri_internal_(f0, f1, f2);
                f1 = f2;
            }
#else
            i0 = hgl_rita_next_vbuf_index_internal_();
            i1 = hgl_rita_next_vbuf_index_internal_();
            if ((i0 == -1) || (i1 == -1)) {
                break;
            }
            v0 = &hgl_rita_ctx__.vertices.vbuf->arr[i0];
            v1 = &hgl_rita_ctx__.vertices.vbuf->arr[i1];
            f0 = hgl_rita_process_vertex_internal_(v0);
            f1 = hgl_rita_process_vertex_internal_(v1);

            for (;;) {
                i2 = hgl_rita_next_vbuf_index_internal_(); if (i2 == -1) { break; }
                v2 = &hgl_rita_ctx__.vertices.vbuf->arr[i2];
                f2 = hgl_rita_process_vertex_internal_(v2);

                hgl_rita_dispatch_tri_internal_(f0, f1, f2);

                f1 = f2;
                v1 = v2;
            }
#endif
        } break;

        default: assert(0 && "Unsupported primitive");
    }
}

static inline void hgl_rita_blit(int x, int y, int w, int h,
                                 HglRitaTexture *src,
                                 HglRitaBlendMethod blend_method,
                                 HglRitaBlitFBMask mask,
                                 HglRitaBlitFBSampler sampling_method,
                                 HglRitaFragShaderFunc shader)
{
    HglRitaAABB blit_aabb = hgl_rita_aabb_make(x, y, w, h);
    HglRitaTileOp op = {
        .blit_info = {
            .aabb          = blit_aabb,
            .texture       = src,
            .blend_method  = blend_method,
            .mask          = mask,
            .sampler       = sampling_method,
            .shader        = shader
        },
        .kind = HGL_RITA_OP_BLIT,
    };

    /* dispatch blit operation to intersecting tiles */
    int fb_w = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->width;
    int fb_h = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->height;
    HglRitaAABB aabb = hgl_rita_aabb_clip(blit_aabb, 0, 0, fb_w - 1, fb_h - 1);
    int start_x = aabb.min_x / HGL_RITA_TILE_SIZE_X;
    int start_y = aabb.min_y / HGL_RITA_TILE_SIZE_Y;
    int end_x = aabb.max_x / HGL_RITA_TILE_SIZE_X + 1;
    int end_y = aabb.max_y / HGL_RITA_TILE_SIZE_Y + 1;
    int stride = hgl_rita_ctx__.renderer.n_tile_cols;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            int i = y*stride + x;
            hgl_rita_queue_push(&(hgl_rita_ctx__.renderer.tile[i].op_queue), op);
        }
    }
}

/*---------------------------------------------------------------------------------------*/
/*--- HglRitaTexture: standalone functions ----------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

static inline HglRitaTexture hgl_rita_texture_make(int width, int height, HglRitaPixelFormat format)
{
    HglRitaTexture tex = (HglRitaTexture) {
        .format     = format,
        .width      = width,
        .height     = height,
        .stride     = width,
    };

    switch (format) {
        case HGL_RITA_RGBA8: {
            tex.data.rgba8 = HGL_RITA_ALLOC(sizeof(HglRitaColor) * width * height);
        } break;
        case HGL_RITA_R32: {
            tex.data.r32   = HGL_RITA_ALLOC(sizeof(float) * width * height);
        } break;
    }

    return tex;
}

static inline void hgl_rita_texture_destroy(HglRitaTexture *tex)
{
    assert((tex->stride == tex->width) && "Trying to free a texture with stride != width. Is this a subtexture?");
    HGL_RITA_FREE(tex->data.rgba8);
    tex->data.rgba8 = NULL;
}

static inline HglRitaTexture hgl_rita_texture_get_subtexture(HglRitaTexture tex,
                                                             int x, int y,
                                                             int width, int height)
{
    bool valid_area = (x >= 0) && (x < tex.width) &&
                      (y >= 0) && (y < tex.height) &&
                      (width <= (tex.width - x)) &&
                      (height <= (tex.height - y));
    assert(valid_area && "Subtexture area is outside the bounds of the original texture.");
    assert((tex.stride == tex.width) && "Trying to take a subtexture of a subtexture. Not supported yet.");

    HglRitaTexture subtex = (HglRitaTexture) {
        .format     = tex.format,
        .width      = width,
        .height     = height,
        .stride     = tex.stride,
    };

    switch (tex.format) {
        case HGL_RITA_RGBA8: {
            subtex.data.rgba8 = tex.data.rgba8 + y*tex.stride + x;
        } break;
        case HGL_RITA_R32: {
            subtex.data.r32 = tex.data.r32 + y*tex.stride + x;
        } break;
    }

    return subtex;
}

static inline void hgl_rita_texture_flip_vertically(HglRitaTexture *tex)
{

    size_t row_size = 0;
    switch (tex->format) {
        case HGL_RITA_RGBA8: {
            row_size = tex->stride * sizeof(HglRitaColor);
        } break;
        case HGL_RITA_R32: {
            row_size = tex->stride * sizeof(float);
        } break;
    }
    static unsigned char temp_row[8192];
    assert((tex->width < 8192) && "Texture is way too big, lmao. This is a toy library.");
    assert((tex->stride == tex->width) && "Trying to vertically flip a subtexture. Not gonna happen.");
    for (int y = 0; y < tex->height / 2; y++) {
        void *upper_row = (void *) &tex->data.rgba8[y * tex->stride];
        void *lower_row = (void *) &tex->data.rgba8[(tex->height - y - 1) * tex->stride];
        memcpy(temp_row, lower_row, row_size);
        memcpy(lower_row, upper_row, row_size);
        memcpy(upper_row, temp_row, row_size);
    }
}

static inline void hgl_rita_texture_blit(HglRitaTexture dst,
                                         HglRitaTexture src,
                                         HglRitaBlendMethod blend_method,
                                         bool flip_vertical)
{
    assert(dst.format == HGL_RITA_RGBA8);
    assert(src.format == HGL_RITA_RGBA8);
    int w = dst.width;
    int h = dst.height;
    int s = dst.stride;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * s + x;
            Vec2 uv = {
                .x = (float) x / (float) w,
                .y = (float) y / (float) h,
            };
            if (flip_vertical) {
                uv.y = 1.0f - uv.y;
            }
            HglRitaColor  src_color = hgl_rita_sample_uv(&src, uv);
            HglRitaColor *dst_color = &dst.data.rgba8[idx];

            *dst_color = hgl_rita_color_blend(*dst_color, src_color, blend_method);
        }
    }
}


/*---------------------------------------------------------------------------------------*/
/*--- HglRitaVertex: standalone functions -----------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

static inline bool hgl_rita_vertex_eq(HglRitaVertex v0, HglRitaVertex v1)
{
    return (v0.pos.x == v1.pos.x) &&
           (v0.pos.y == v1.pos.y) &&
           (v0.pos.z == v1.pos.z) &&
           (v0.pos.w == v1.pos.w) &&
           (v0.normal.x == v1.normal.x) &&
           (v0.normal.y == v1.normal.y) &&
           (v0.normal.z == v1.normal.z) &&
#ifndef HGL_RITA_SIMPLE
           (v0.tangent.x == v1.tangent.x) &&
           (v0.tangent.y == v1.tangent.y) &&
           (v0.tangent.z == v1.tangent.z) &&
#endif
           (v0.uv.x == v1.uv.x) &&
           (v0.uv.y == v1.uv.y) &&
           (v0.color.r == v1.color.r) &&
           (v0.color.g == v1.color.g) &&
           (v0.color.b == v1.color.b) &&
           (v0.color.a == v1.color.a);
}


/*---------------------------------------------------------------------------------------*/
/*--- HglRitaColor: standalone functions ------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

static inline HglRitaColor hgl_rita_color_blend(HglRitaColor c0,
                                                HglRitaColor c1,
                                                HglRitaBlendMethod method)
{
    HglRitaColor color;
    switch (method) {
        case HGL_RITA_REPLACE: {
            color = c1;
        } break;

        case HGL_RITA_REPLACE_SKIP_ALPHA: {
            color = c1;
            color.a = c0.a;
        } break;

        case HGL_RITA_ADD: {
            color = hgl_rita_color_add(c0, c1);
        } break;

        case HGL_RITA_SUBTRACT: {
            color = hgl_rita_color_sub(c0, c1);
        } break;

        case HGL_RITA_SUBTRACT_SKIP_ALPHA: {
            color = hgl_rita_color_sub(c0, c1);
            color.a = c0.a;
        } break;

        case HGL_RITA_MULTIPLY: {
            color = hgl_rita_color_mul(c0, c1);
        } break;

        case HGL_RITA_ALPHA: {
            color = hgl_rita_color_lerp(c0, c1, (float)c1.a/255.0f);
            color.a = 255;
        } break;

        case HGL_RITA_ONE_MINUS_ALPHA: {
            color = hgl_rita_color_lerp(c0, c1, 1.0f - (float)c1.a/255.0f);
            color.a = 255;
        } break;
    }

    return color;
}

static inline HglRitaColor hgl_rita_color_lerp(HglRitaColor c0, HglRitaColor c1, float t)
{
#if 1
    const int num = t*0x100;
    return (HglRitaColor) {
        .r = ((c0.r << 8) - (c0.r * num) + (c1.r * num)) >> 8,
        .g = ((c0.g << 8) - (c0.g * num) + (c1.g * num)) >> 8,
        .b = ((c0.b << 8) - (c0.b * num) + (c1.b * num)) >> 8,
        .a = ((c0.a << 8) - (c0.a * num) + (c1.a * num)) >> 8,
    };
#else
    return (HglRitaColor) {
        .r = ((c0.r/255.0f) * (1.0f - t) + (c1.r/255.0f) * t) * 255.0f,
        .g = ((c0.g/255.0f) * (1.0f - t) + (c1.g/255.0f) * t) * 255.0f,
        .b = ((c0.b/255.0f) * (1.0f - t) + (c1.b/255.0f) * t) * 255.0f,
        .a = ((c0.a/255.0f) * (1.0f - t) + (c1.a/255.0f) * t) * 255.0f,
    };
#endif
}

static inline HglRitaColor hgl_rita_color_add(HglRitaColor c0, HglRitaColor c1)
{
    return (HglRitaColor) {
        .r = clamp(0, 255, c0.r + c1.r),
        .g = clamp(0, 255, c0.g + c1.g),
        .b = clamp(0, 255, c0.b + c1.b),
        .a = clamp(0, 255, c0.a + c1.a),
    };
}

static inline HglRitaColor hgl_rita_color_sub(HglRitaColor c0, HglRitaColor c1)
{
    return (HglRitaColor) {
        .r = clamp(0, 255, c0.r - c1.r),
        .g = clamp(0, 255, c0.g - c1.g),
        .b = clamp(0, 255, c0.b - c1.b),
        .a = clamp(0, 255, c0.a - c1.a),
    };
}

static inline HglRitaColor hgl_rita_color_mul(HglRitaColor c0, HglRitaColor c1)
{
    return (HglRitaColor) {
        .r = (c0.r/255.0f) * (c1.r/255.0f) * 255.0f,
        .g = (c0.g/255.0f) * (c1.g/255.0f) * 255.0f,
        .b = (c0.b/255.0f) * (c1.b/255.0f) * 255.0f,
        .a = (c0.a/255.0f) * (c1.a/255.0f) * 255.0f,
    };
}

static inline HglRitaColor hgl_rita_color_mul_scalar(HglRitaColor c0, float s)
{
    return (HglRitaColor) {
        .r = clamp(0, 255, c0.r * s),
        .g = clamp(0, 255, c0.g * s),
        .b = clamp(0, 255, c0.b * s),
        .a = clamp(0, 255, c0.a * s),
    };
}

static inline bool hgl_rita_color_eq(HglRitaColor c0, HglRitaColor c1)
{
    return (c0.r == c1.r) &&
           (c0.g == c1.g) &&
           (c0.b == c1.b) &&
           (c0.a == c1.a);
}

static inline float hgl_rita_color_luminance(HglRitaColor c0)
{
    float r = (float)c0.r / 255.0f;
    float g = (float)c0.g / 255.0f;
    float b = (float)c0.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

static inline Vec4 hgl_rita_color_as_vector(HglRitaColor c0)
{
    return (Vec4) {
        .x = (float) c0.r / 255.0f,
        .y = (float) c0.g / 255.0f,
        .z = (float) c0.b / 255.0f,
        .w = (float) c0.a / 255.0f,
    };
}

static inline HglRitaColor hgl_rita_color_from_vector(Vec4 v)
{
    v.x = clamp(0.0f, 1.0f, v.x);
    v.y = clamp(0.0f, 1.0f, v.y);
    v.z = clamp(0.0f, 1.0f, v.z);
    v.w = clamp(0.0f, 1.0f, v.w);
    return (HglRitaColor) {
        .r = 255 * v.x,
        .g = 255 * v.y,
        .b = 255 * v.z,
        .a = 255 * v.w,
    };
}


/*---------------------------------------------------------------------------------------*/
/*--- HglRitaAABB: standalone functions -------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

static inline HglRitaAABB hgl_rita_aabb_make(int x, int y, int w, int h)
{
    return (HglRitaAABB) {
        .min_x = x,
        .min_y = y,
        .max_x = x + w,
        .max_y = y + h,
    };
}

static inline HglRitaAABB hgl_rita_aabb_from_line(HglRitaLine line)
{
    HglRitaFragment f0 = line.f0;
    HglRitaFragment f1 = line.f1;

    int temp;
    int y_min = f0.y;
    int y_max = f1.y;
    int x_min = f0.x;
    int x_max = f1.x;

    if (y_min > y_max) {
        temp = y_min;
        y_min = y_max;
        y_max = temp;
    }

    if (x_min > x_max) {
        temp = x_min;
        x_min = x_max;
        x_max = temp;
    }

    return (HglRitaAABB) {
        .min_x = x_min,
        .min_y = y_min,
        .max_x = x_max,
        .max_y = y_max,
    };
}

static inline HglRitaAABB hgl_rita_aabb_from_tri(HglRitaTriangle tri)
{
    HglRitaFragment f0 = tri.f0;
    HglRitaFragment f1 = tri.f1;
    HglRitaFragment f2 = tri.f2;

    int y_min = f0.y;
    if (f1.y < y_min) y_min = f1.y;
    if (f2.y < y_min) y_min = f2.y;
    int y_max = f0.y;
    if (f1.y > y_max) y_max = f1.y;
    if (f2.y > y_max) y_max = f2.y;
    int x_min = f0.x;
    if (f1.x < x_min) x_min = f1.x;
    if (f2.x < x_min) x_min = f2.x;
    int x_max = f0.x;
    if (f1.x > x_max) x_max = f1.x;
    if (f2.x > x_max) x_max = f2.x;

    return (HglRitaAABB) {
        .min_x = x_min,
        .min_y = y_min,
        .max_x = x_max,
        .max_y = y_max,
    };
}

static inline HglRitaAABB hgl_rita_aabb_clip(HglRitaAABB aabb, int min_x, int min_y, int max_x, int max_y)
{
    return (HglRitaAABB) {
        .min_x = max(min_x, aabb.min_x),
        .min_y = max(min_y, aabb.min_y),
        .max_x = min(max_x, aabb.max_x),
        .max_y = min(max_y, aabb.max_y),
    };
}

static inline HglRitaAABB hgl_rita_aabb_intersection(HglRitaAABB a, HglRitaAABB b)
{
    return (HglRitaAABB) {
        .min_x = max(a.min_x, b.min_x),
        .min_y = max(a.min_y, b.min_y),
        .max_x = min(a.max_x, b.max_x),
        .max_y = min(a.max_y, b.max_y),
    };
}

static inline bool hgl_rita_aabb_intersects(HglRitaAABB a, HglRitaAABB b)
{
    return (a.min_x <= b.max_x && a.max_x >= b.min_x) &&
           (a.min_y <= b.max_y && a.max_y >= b.min_y);
}

/*---------------------------------------------------------------------------------------*/
/*--- Texture sampling ------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

static inline HglRitaColor hgl_rita_sample(HglRitaTexture *tex, int x, int y)
{
    // TODO respect wrapping mode
    if (tex == NULL) {
        return HGL_RITA_MAGENTA;
    }

    HglRitaColor color = {0};
    x = clamp(0, tex->width - 1, x);
    y = clamp(0, tex->height - 1, y);
    int idx = y * tex->stride + x;
    switch (tex->format) {
        case HGL_RITA_RGBA8: {
            color = tex->data.rgba8[idx];
        } break;
        case HGL_RITA_R32: {
            color.r = 255*tex->data.r32[idx];
        } break;
    }
    return color;
}

static inline HglRitaColor hgl_rita_sample_uv(HglRitaTexture *tex, Vec2 uv)
{
    if (tex == NULL) {
        return HGL_RITA_MAGENTA;
    }

    assert(tex->format == HGL_RITA_RGBA8 && "format not supported yet");
    assert(tex->stride != 0 && "Texture has a stride of 0. ");
    HglRitaColor color;
    const float BIAS = 0.001f;
    int w = tex->width;
    int h = tex->height;
    int s = tex->stride;

    switch (hgl_rita_ctx__.opts.texture_wrapping) {
        case HGL_RITA_NO_WRAPPING: break;
        case HGL_RITA_CLAMP: {
            uv.x = clamp(0.0f, 1.0f, uv.x);
            uv.y = clamp(0.0f, 1.0f, uv.y);
        } break;
        case HGL_RITA_REPEAT: {
            float discard;
            uv.x = modff(uv.x, &discard);
            uv.y = modff(uv.y, &discard);
            if (signbit(uv.x)) uv.x = 1.0f + uv.x;
            if (signbit(uv.y)) uv.y = 1.0f + uv.y;
            // I guess lol.
            //uv.x = ((*(uint32_t *)&uv.x) >> 31) + uv.x;
            //uv.y = ((*(uint32_t *)&uv.y) >> 31) + uv.y;
        } break;
    }

    switch (hgl_rita_ctx__.opts.texture_filter) {
        case HGL_RITA_NEAREST: {
            int x = uv.x * (w - BIAS);
            int y = uv.y * (h - BIAS);
            //int x = uv.x * w;
            //int y = uv.y * h;
            switch (tex->format) {
                case HGL_RITA_RGBA8: {
                    color = tex->data.rgba8[y*s + x];
                } break;
                case HGL_RITA_R32: {
                    color = HGL_RITA_BLACK;
                    color.r = 255*tex->data.r32[y*s + x];
                } break;
            }
        } break;

        case HGL_RITA_BILINEAR: {
            //float x = uv.x * w;
            //float y = uv.y * h;
            float x = uv.x * (w - 1.0f - BIAS);
            float y = uv.y * (h - 1.0f - BIAS);
            int l = (int) x;
            int r = l + 1;
            int t = (int) y;
            int b = t + 1;
            float t_x = x - l;
            float t_y = y - t;
            switch (tex->format) {
                case HGL_RITA_RGBA8: {
                    HglRitaColor ul, ur, ll, lr, left, right;
                    ul = tex->data.rgba8[t*s + l];
                    ur = tex->data.rgba8[t*s + r];
                    ll = tex->data.rgba8[b*s + l];
                    lr = tex->data.rgba8[b*s + r];
                    left = hgl_rita_color_lerp(ul, ll, t_y);
                    right = hgl_rita_color_lerp(ur, lr, t_y);
                    color = hgl_rita_color_lerp(left, right, t_x);
                } break;
                case HGL_RITA_R32: {
                    float ul, ur, ll, lr, left, right;
                    ul = tex->data.r32[t*s + l];
                    ur = tex->data.r32[t*s + r];
                    ll = tex->data.r32[b*s + l];
                    lr = tex->data.r32[b*s + r];
                    left = lerp(ul, ll, t_y);
                    right = lerp(ur, lr, t_y);
                    color = HGL_RITA_BLACK;
                    color.r = 255*lerp(left, right, t_x);
                } break;
            }
        } break;
    }

    return color;
}

static inline HglRitaColor hgl_rita_sample_rectilinear(HglRitaTexture *tex, Vec3 dir)
{
    Vec2 uv;
    uv.x = atan2f(dir.z, dir.x) / (2.0f * PI) + 0.5f;
    uv.y = dir.y * 0.5f + 0.5f;
    return hgl_rita_sample_uv(tex, uv);
}

static inline HglRitaColor hgl_rita_sample_cubemap(HglRitaTexture *tex, Vec3 dir)
{
    /*
     * Assumes the cubemap texture in `tex` is laid out like this:
     *
     *        +----+
     *        | T  |
     *   +----+----+----+----+
     *   | L  | F  | R  | B  |
     *   +----+----+----+----+
     *        | B  |
     *        +----+
     */
    uint32_t major_axis = 1; // x=1, y=2, z=4, -x=8, -y=16, -z=32
    float abs_x = fabsf(dir.x);
    float abs_y = fabsf(dir.y);
    float abs_z = fabsf(dir.z);
    if (abs_x >= abs_y && abs_x >= abs_z) major_axis = 1; else
    if (abs_y >= abs_x && abs_y >= abs_z) major_axis = 2; else
    if (abs_z >= abs_x && abs_z >= abs_y) major_axis = 4;
    if ((major_axis & 1) && dir.x < 0) major_axis <<= 3; else
    if ((major_axis & 2) && dir.y < 0) major_axis <<= 3; else
    if ((major_axis & 4) && dir.z < 0) major_axis <<= 3;
    const int x_step = tex->width / 4;
    const int y_step = tex->height / 3;

    Vec2 uv;
    HglRitaTexture dir_subtex;
    switch(major_axis) {
        case 1: {
            uv.x = 0.5f * (dir.z / dir.x) + 0.5f;
            uv.y = 0.5f * (dir.y / dir.x) + 0.5f;
            dir_subtex = hgl_rita_texture_get_subtexture(*tex, 2*x_step, 1*y_step, x_step, y_step);
        } break;
        case 2: {
            uv.x = 0.5f * (dir.x / dir.y) + 0.5f;
            uv.y = 0.5f * (dir.z / dir.y) + 0.5f;
            dir_subtex = hgl_rita_texture_get_subtexture(*tex, 1*x_step, 2*y_step, x_step, y_step);
        } break;
        case 4: {
            uv.x = 0.5f * (-dir.x / dir.z) + 0.5f;
            uv.y = 0.5f * (dir.y / dir.z) + 0.5f;
            dir_subtex = hgl_rita_texture_get_subtexture(*tex, 3*x_step, 1*y_step, x_step, y_step);
        } break;
        case 8: {
            uv.x = 0.5f * (dir.z / dir.x) + 0.5f;
            uv.y = 0.5f * (-dir.y / dir.x) + 0.5f;
            dir_subtex = hgl_rita_texture_get_subtexture(*tex, 0*x_step, 1*y_step, x_step, y_step);
        } break;
        case 16: {
            uv.x = 0.5f * (-dir.x / dir.y) + 0.5f;
            uv.y = 0.5f * (dir.z / dir.y) + 0.5f;
            dir_subtex = hgl_rita_texture_get_subtexture(*tex, 1*x_step, 0*y_step, x_step, y_step);
        } break;
        case 32: {
            uv.x = 0.5f * (-dir.x / dir.z) + 0.5f;
            uv.y = 0.5f * (-dir.y / dir.z) + 0.5f;
            dir_subtex = hgl_rita_texture_get_subtexture(*tex, 1*x_step, 1*y_step, x_step, y_step);
        } break;
        default: return HGL_RITA_CYAN; // shouldn't happen
    }

    return hgl_rita_sample_uv(&dir_subtex, uv);
}

static inline HglRitaColor hgl_rita_sample_unit(HglRitaTexUnit unit, int x, int y)
{
    return hgl_rita_sample(hgl_rita_ctx__.tex_unit[unit], x, y);
}

static inline HglRitaColor hgl_rita_sample_unit_uv(HglRitaTexUnit unit, Vec2 uv)
{
    return hgl_rita_sample_uv(hgl_rita_ctx__.tex_unit[unit], uv);
}

static inline HglRitaColor hgl_rita_sample_unit_rectilinear(HglRitaTexUnit unit, Vec3 dir)
{
    return hgl_rita_sample_rectilinear(hgl_rita_ctx__.tex_unit[unit], dir);
}

static inline HglRitaColor hgl_rita_sample_unit_cubemap(HglRitaTexUnit unit, Vec3 dir)
{
    return hgl_rita_sample_cubemap(hgl_rita_ctx__.tex_unit[unit], dir);
}


/*---------------------------------------------------------------------------------------*/
/*--- Internal functions ----------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

static inline void *hgl_rita_tile_thread_internal_(void *arg)
{
    nice(10);

    HglRitaTile *tile = (HglRitaTile *) arg;
    HglRitaTileOpQueue *q = &tile->op_queue;
    HglRitaAABB tile_aabb = tile->aabb;

    for (;;) {

        HglRitaTileOp op = hgl_rita_queue_pop(q, HglRitaTileOp);

        switch (op.kind) {

            /**
             * Triangles
             *
             * TODO top left bias?
             */
            case HGL_RITA_OP_RASTERIZE_TRIANGLE: {
                HglRitaFragment f0 = op.triangle.f0;
                HglRitaFragment f1 = op.triangle.f1;
                HglRitaFragment f2 = op.triangle.f2;

                HglRitaAABB aabb = hgl_rita_aabb_intersection(hgl_rita_aabb_from_tri(op.triangle), tile_aabb);
                //aabb.max_x--;
                //aabb.max_y--;

                float det = hgl_rita_det_internal_(f2.x, f2.y, f1.x, f1.y, f0.x, f0.y);
                if (fabsf(det) < 0.001f) break;
                float r_area = 1.0f / hgl_rita_det_internal_(f2.x, f2.y, f1.x, f1.y, f0.x, f0.y);
                bool frontfacing = r_area < 0;

                float delta_w0_col = (f2.y - f1.y);
                float delta_w1_col = (f0.y - f2.y);
                float delta_w2_col = (f1.y - f0.y);
                float delta_w0_row = (f1.x - f2.x);
                float delta_w1_row = (f2.x - f0.x);
                float delta_w2_row = (f0.x - f1.x);

                int x = aabb.min_x;
                int y = aabb.min_y;
                float w0_row = hgl_rita_det_internal_(x, y, f1.x, f1.y, f2.x, f2.y); // + bias0;
                float w1_row = hgl_rita_det_internal_(f0.x, f0.y, x, y, f2.x, f2.y); // + bias1;
                float w2_row = hgl_rita_det_internal_(f0.x, f0.y, f1.x, f1.y, x, y); // + bias2;

                for (int y = aabb.min_y; y < aabb.max_y; y++) {
                    float w0 = w0_row;
                    float w1 = w1_row;
                    float w2 = w2_row;
                    for (int x = aabb.min_x; x < aabb.max_x; x++) {
                        bool is_inside;
                        if (frontfacing) {
                            is_inside = w0 >= 0 && w1 >= 0 && w2 >= 0;
                        } else {
                            is_inside = w0 <= 0 && w1 <= 0 && w2 <= 0;
                        }

                        if (is_inside) {
                            float u = -w0 * r_area;
                            float v = -w1 * r_area;

                            HglRitaFragment frag = hgl_rita_frag_berp_internal_(f0, f1, f2, u, v, x, y);
                            hgl_rita_process_fragment_internal_(&frag);
                        }

                        w0 += delta_w0_col;
                        w1 += delta_w1_col;
                        w2 += delta_w2_col;
                    }

                    w0_row += delta_w0_row;
                    w1_row += delta_w1_row;
                    w2_row += delta_w2_row;
                }
            } break;

            /**
             * Lines
             */
            case HGL_RITA_OP_RASTERIZE_LINE: {
                HglRitaFragment f0 = op.line.f0;
                HglRitaFragment f1 = op.line.f1;

                /* Cohen-Sutherland clip to AABB of tile */
                const int INSIDE = 0b0000;
                const int LEFT   = 0b0001;
                const int RIGHT  = 0b0010;
                const int BOTTOM = 0b0100;
                const int TOP    = 0b1000;

                int x0 = f0.x;
                int y0 = f0.y;
                int x1 = f1.x;
                int y1 = f1.y;

                int outcode0 = INSIDE;
                if      (x0 < tile_aabb.min_x) outcode0 |= LEFT;
                else if (x0 > tile_aabb.max_x) outcode0 |= RIGHT;
                if      (y0 < tile_aabb.min_y) outcode0 |= BOTTOM;
                else if (y0 > tile_aabb.max_y) outcode0 |= TOP;

                int outcode1 = INSIDE;
                if      (x1 < tile_aabb.min_x) outcode1 |= LEFT;
                else if (x1 > tile_aabb.max_x) outcode1 |= RIGHT;
                if      (y1 < tile_aabb.min_y) outcode1 |= BOTTOM;
                else if (y1 > tile_aabb.max_y) outcode1 |= TOP;

                bool accept = false;
                while (true) {
                    if (0 == (outcode0 | outcode1)) {
                        /* trivial accept */
                        accept = true;
                        break;
                    } else if (0 != (outcode0 & outcode1)) {
                        /* trivial reject */
                        break;
                    } else {
                        /* non-trivial case: clip line */
                        int x;
                        int y;

                        /* Pick any outside point */
                        int outside_outcode = outcode0 > outcode1 ? outcode0 : outcode1;

                        /* find intersection point */
                        if ((outside_outcode & TOP) != 0) {
                            x = x0 + (x1 - x0) * (tile_aabb.max_y - y0) / (y1 - y0);
                            y = tile_aabb.max_y;
                        } else if ((outside_outcode & BOTTOM) != 0) {
                            x = x0 + (x1 - x0) * (tile_aabb.min_y - y0) / (y1 - y0);
                            y = tile_aabb.min_y;
                        } else if ((outside_outcode & RIGHT) != 0) {
                            y = y0 + (y1 - y0) * (tile_aabb.max_x - x0) / (x1 - x0);
                            x = tile_aabb.max_x;
                        } else /* LEFT */ {
                            y = y0 + (y1 - y0) * (tile_aabb.min_x - x0) / (x1 - x0);
                            x = tile_aabb.min_x;
                        }

                        if (outside_outcode == outcode0) {
                            x0 = x;
                            y0 = y;
                                                        outcode0 = INSIDE;
                                                        if      (x0 < tile_aabb.min_x) outcode0 |= LEFT;
                                                        else if (x0 > tile_aabb.max_x) outcode0 |= RIGHT;
                                                        if      (y0 < tile_aabb.min_y) outcode0 |= BOTTOM;
                                                        else if (y0 > tile_aabb.max_y) outcode0 |= TOP;
                        } else {
                            x1 = x;
                            y1 = y;
                                                        outcode1 = INSIDE;
                                                        if      (x1 < tile_aabb.min_x) outcode1 |= LEFT;
                                                        else if (x1 > tile_aabb.max_x) outcode1 |= RIGHT;
                                                        if      (y1 < tile_aabb.min_y) outcode1 |= BOTTOM;
                                                        else if (y1 > tile_aabb.max_y) outcode1 |= TOP;
                                                }
                    }
                }

                /* line was not accepted (outside AABB) */
                                if (!accept) {
                    break;
                }

                /* recalculate fragments */
                float t;
                int dx = x1 - x0;
                int dy = y1 - y0;
                if (abs(dx) > abs(dy)) {
                    t = (float)(x0 - f0.x) / (float)(f1.x - f0.x);
                    f0 = hgl_rita_frag_lerp_internal_(x0, y0, f0, f1, t);
                    t = (float)(x1 - f0.x) / (float)(f1.x - f0.x);
                    f1 = hgl_rita_frag_lerp_internal_(x1, y1, f0, f1, t);
                } else {
                    t = (float)(y0 - f0.y) / (float)(f1.y - f0.y);
                    f0 = hgl_rita_frag_lerp_internal_(x0, y0, f0, f1, t);
                    t = (float)(y1 - f0.y) / (float)(f1.y - f0.y);
                    f1 = hgl_rita_frag_lerp_internal_(x1, y1, f0, f1, t);
                }

                /* swap so that x0 < x1 */
                HglRitaFragment temp_frag;
                if (f0.x > f1.x) {
                    temp_frag = f0; f0 = f1; f1 = temp_frag;
                }

                dx = (int)f1.x - (int)f0.x;
                dy = (int)f1.y - (int)f0.y;

                if (dx > abs(dy)) {
                    float y_step = (float)dy / (float)dx;
                    for (int i = 0; i < dx; i++) {
                        float t = (float) i / (float) dx;
                        int x = f0.x + i;
                        int y = f0.y + i*y_step;
                        HglRitaFragment frag = hgl_rita_frag_lerp_internal_(x, y, f0, f1, t);
                        hgl_rita_process_fragment_internal_(&frag);
                    }
                } else {
                    /* swap so we iterate on y in the positive direction */
                    if (dy < 0) {
                        temp_frag = f0; f0 = f1; f1 = temp_frag;
                    }

                    float x_step = (float)dx / (float)dy;
                    for (int i = 0; i < abs(dy); i++) {
                        float t = (float) i / (float) abs(dy);
                        int x = f0.x + i*x_step;
                        int y = f0.y + i;
                        HglRitaFragment frag = hgl_rita_frag_lerp_internal_(x, y, f0, f1, t);
                        hgl_rita_process_fragment_internal_(&frag);
                    }
                }
            } break;

            /**
             * Points/Pixels
             */
            case HGL_RITA_OP_RASTERIZE_POINT: {
                HglRitaFragment f0 = op.line.f0;
                hgl_rita_process_fragment_internal_(&f0); // a bit more straight forward this time
            } break;

            /**
             * Vertex processing
             */
            case HGL_RITA_OP_PROCESS_VBUF_SEGMENT: {
#ifdef HGL_RITA_PARALLEL_VERTEX_PROCESSING
                int start = op.vbuf_segment.start_idx;
                int end = op.vbuf_segment.end_idx;
                for (int i = start; i < end; i++) {
                    const HglRitaVertex *v = &hgl_rita_ctx__.vertices.vbuf->arr[i];
                    hgl_rita_ctx__.vertices.fbuf.arr[i] = hgl_rita_process_vertex_internal_(v);
                }
#endif
            } break;

            /**
             * Blit
             */
            case HGL_RITA_OP_BLIT: {
                HglRitaTexture *fb = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER];
                HglRitaTexture *db = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER];

                HglRitaTexture *src                  = op.blit_info.texture;
                HglRitaBlendMethod blend_method      = op.blit_info.blend_method;
                HglRitaBlitFBMask mask               = op.blit_info.mask;
                HglRitaBlitFBSampler sampling_method = op.blit_info.sampler;

                int fb_w = fb->width;
                int fb_h = fb->height;
                int fb_s = fb->stride;

                //Mat4 view_to_world_dir;
                //float z = hgl_rita_ctx__.tform.proj.m11;

                //if ((sampling_method == HGL_RITA_VIEW_DIR_CUBEMAP) ||
                //    (sampling_method == HGL_RITA_VIEW_DIR_RECTILINEAR)) {
                //    view_to_world_dir = hgl_rita_ctx__.tform.view;
                //    view_to_world_dir.c3.x = 0;
                //    view_to_world_dir.c3.y = 0;
                //    view_to_world_dir.c3.z = 0;
                //    view_to_world_dir = mat4_transpose(view_to_world_dir);
                //}

                HglRitaAABB aabb = hgl_rita_aabb_intersection(tile_aabb, op.blit_info.aabb);
                int x = aabb.min_x;
                int y = aabb.min_y;
                int w = aabb.max_x - aabb.min_x;
                int h = aabb.max_y - aabb.min_y;
                int box_w = op.blit_info.aabb.max_x - op.blit_info.aabb.min_x - 1;
                int box_h = op.blit_info.aabb.max_y - op.blit_info.aabb.min_y - 1;

                for (int j = 0; j < h; j++) {
                    for (int i = 0; i < w; i++) {
                        int screen_y =  y + j;
                        int screen_x =  x + i;
                        int box_y = screen_y - op.blit_info.aabb.min_y;
                        int box_x = screen_x - op.blit_info.aabb.min_x;
                        HglRitaColor src_color = HGL_RITA_BLACK;
                        HglRitaColor *dst_color = NULL;
                        float *dst_depth = NULL;

                        int idx = screen_y * fb_s + screen_x;
                        dst_color = &fb->data.rgba8[idx];

                        switch (mask) {
                            case HGL_RITA_EVERYWHERE: break;

                            case HGL_RITA_CLEAR_COLOR: {
                                if (!hgl_rita_color_eq(*dst_color, hgl_rita_ctx__.opts.clear_color)) {
                                    continue;
                                }
                            } break;

                            case HGL_RITA_NON_CLEAR_COLOR: {
                                if (hgl_rita_color_eq(*dst_color, hgl_rita_ctx__.opts.clear_color)) {
                                    continue;
                                }
                            } break;

                            case HGL_RITA_DEPTH_INF: {
                                assert(db != NULL && "Missing depth attachment in framebuffer (Note: Needed by HGL_RITA_DEPTH_INF mask)");
                                dst_depth = &db->data.r32[idx];
                                if (*dst_depth != 1.0f) {
                                    continue;
                                }
                            } break;

                            case HGL_RITA_DEPTH_NON_INF: {
                                assert(db != NULL && "Missing depth attachment in framebuffer (Note: Needed by HGL_RITA_DEPTH_NON_INF mask)");
                                dst_depth = &db->data.r32[idx];
                                if (*dst_depth != 1.0f) {
                                    continue;
                                }
                            } break;
                        }

                        switch (sampling_method) {
                            case HGL_RITA_BOXCOORD: {
                                Vec2 uv = (Vec2) {
                                    (float)box_x / (float)box_w,
                                    ((float)box_y / (float)box_h),
                                };
                                src_color = hgl_rita_sample_uv(src, uv);
                            } break;

                            case HGL_RITA_SCREENCOORD: {
                                Vec2 uv = (Vec2) {
                                    (float)screen_x / (float)fb_w,
                                    ((float)screen_y / (float)fb_h),
                                };
                                src_color = hgl_rita_sample_uv(src, uv);
                            } break;

                            case HGL_RITA_VIEW_DIR_RECTILINEAR: {
                                float sn_x = 2.0f*((float)screen_x / (float)fb_w) - 1.0f;
                                float sn_y = 2.0f*((float)screen_y / (float)fb_h) - 1.0f;
                                float z = hgl_rita_ctx__.tform.proj.m11;
                                Vec3 dir = vec3_make(hgl_rita_ctx__.tform.camera.aspect * sn_x, -sn_y, -z);
                                dir = vec3_normalize(dir);
                                dir = mat3_mul_vec3(hgl_rita_ctx__.tform.iview, dir);
                                src_color = hgl_rita_sample_rectilinear(src, dir);
                            } break;

                            case HGL_RITA_VIEW_DIR_CUBEMAP: {
                                float sn_x = 2.0f*((float)screen_x / (float)fb_w) - 1.0f;
                                float sn_y = 2.0f*((float)screen_y / (float)fb_h) - 1.0f;
                                float z = hgl_rita_ctx__.tform.proj.m11;
                                Vec3 dir = vec3_make(hgl_rita_ctx__.tform.camera.aspect * sn_x, -sn_y, -z);
                                //dir = vec3_normalize(dir); // not needed
                                dir = mat3_mul_vec3(hgl_rita_ctx__.tform.iview, dir);
                                src_color = hgl_rita_sample_cubemap(src, dir);
                            } break;

                            case HGL_RITA_SHADER: {
                                HglRitaFragment frag;
                                frag.x = screen_x;
                                frag.y = screen_y;
                                frag.inv_z = (db != NULL) ? db->data.r32[idx] : 0.0f;
                                frag.uv = (Vec2) {
                                    //(float)screen_x / (float)(fb_w - 1),
                                    //((float)screen_y / (float)(fb_h - 1)),
                                    (float)box_x / (float)(box_w),
                                    ((float)box_y / (float)(box_h)),
                                };
                                frag.color = hgl_rita_sample_uv(src, frag.uv);
                                src_color = op.blit_info.shader(&hgl_rita_ctx__, &frag);
                            } break;
                        }

                        *dst_color = hgl_rita_color_blend(*dst_color, src_color, blend_method);
                    }
                }
            } break;

            case HGL_RITA_OP_TERMINATE: {
                return NULL;
            } break;
        }
    }
}

static inline void hgl_rita_dispatch_point_internal_(HglRitaFragment f0)
{
    HglRitaTileOp op = {
        .point = {f0},
        .kind = HGL_RITA_OP_RASTERIZE_POINT,
    };

    /* discard clipping */
    if (f0.clipping) {
        return;
    }

    /* dispatch point primitive to intersecting tile */
    int x = f0.x / HGL_RITA_TILE_SIZE_X;
    int y = f0.y / HGL_RITA_TILE_SIZE_Y;
    int stride = hgl_rita_ctx__.renderer.n_tile_cols;
    int i = y*stride + x;
    hgl_rita_queue_push(&(hgl_rita_ctx__.renderer.tile[i].op_queue), op);
}

static inline void hgl_rita_dispatch_line_internal_(HglRitaFragment f0, HglRitaFragment f1)
{
    HglRitaTileOp op = {
        .line = {f0, f1},
        .kind = HGL_RITA_OP_RASTERIZE_LINE,
    };

    /* discard clipping */
    //if ((f0.clipping) &&
    //    (f1.clipping) ) {
    //    return;
    //}

    /* dispatch line primitive to intersecting tiles */
    int w = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->width;
    int h = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->height;
    HglRitaAABB aabb = hgl_rita_aabb_clip(hgl_rita_aabb_from_line(op.line), 0, 0, w - 1, h - 1);
    int start_x = aabb.min_x / HGL_RITA_TILE_SIZE_X;
    int start_y = aabb.min_y / HGL_RITA_TILE_SIZE_Y;
    int end_x = aabb.max_x / HGL_RITA_TILE_SIZE_X + 1;
    int end_y = aabb.max_y / HGL_RITA_TILE_SIZE_Y + 1;
    int stride = hgl_rita_ctx__.renderer.n_tile_cols;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            int i = y*stride + x;
            hgl_rita_queue_push(&(hgl_rita_ctx__.renderer.tile[i].op_queue), op);
        }
    }
}

static inline void hgl_rita_dispatch_tri_internal_(HglRitaFragment f0, HglRitaFragment f1, HglRitaFragment f2)
{
    if (hgl_rita_ctx__.opts.draw_wire_frames) {
        hgl_rita_dispatch_line_internal_(f0, f1);
        hgl_rita_dispatch_line_internal_(f1, f2);
        hgl_rita_dispatch_line_internal_(f2, f0);
        return;
    }

    HglRitaTileOp op = {
        .triangle = {f0, f1, f2},
        .kind = HGL_RITA_OP_RASTERIZE_TRIANGLE,
    };

    /* discard clipping (not completely valid to do this, but hey) */
    if ((f0.clipping) &&
        (f1.clipping) &&
        (f2.clipping)) {
        return;
    }

    /* cull back-facing triangles */
    if (hgl_rita_ctx__.opts.backface_culling_enabled) {
        float det = hgl_rita_det_internal_(f0.x, f0.y, f1.x, f1.y, f2.x, f2.y);
        bool frontfacing = (hgl_rita_ctx__.opts.frontface_winding == HGL_RITA_CCW) ? (det > 0) : (det < 0);
        if (!frontfacing) {
            return;
        }
    }

    /* dispatch triangle primitive to intersecting tiles */
    int w = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->width;
    int h = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->height;
    HglRitaAABB aabb = hgl_rita_aabb_clip(hgl_rita_aabb_from_tri(op.triangle), 0, 0, w - 1, h - 1);
    int start_x = aabb.min_x / HGL_RITA_TILE_SIZE_X;
    int start_y = aabb.min_y / HGL_RITA_TILE_SIZE_Y;
    int end_x = aabb.max_x / HGL_RITA_TILE_SIZE_X + 1;
    int end_y = aabb.max_y / HGL_RITA_TILE_SIZE_Y + 1;
    int stride = hgl_rita_ctx__.renderer.n_tile_cols;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            int i = y*stride + x;
            hgl_rita_queue_push(&(hgl_rita_ctx__.renderer.tile[i].op_queue), op);
        }
    }
}

static inline HglRitaFragment hgl_rita_process_vertex_internal_(const HglRitaVertex *in)
{
    HglRitaVertex vert_out;
    HglRitaFragment frag_out = {0};
    Vec4 v_ls;
    Vec4 v_cs;
    Vec4 v_ndc;
    Vec4 v_ss;

    /* get local space vertex pos and extend to 4D */
    v_ls = in->pos;
    v_ls.w = 1.0f;

    /* vertex shader */
    if (hgl_rita_ctx__.shaders.vert == NULL) {
        Mat4 m_mvp     = hgl_rita_ctx__.tform.mvp;
        Mat3 m_normals = hgl_rita_ctx__.tform.normals;

        vert_out.pos     = mat4_mul_vec4(m_mvp, v_ls);
        vert_out.normal  = mat3_mul_vec3(m_normals, in->normal);
#ifndef HGL_RITA_SIMPLE
        vert_out.tangent = mat3_mul_vec3(m_normals, in->tangent);
#endif
        vert_out.uv      = in->uv;
        vert_out.color   = in->color;
    } else {
        vert_out = hgl_rita_ctx__.shaders.vert(&hgl_rita_ctx__, in);
    }

    /* clip space -> NDC space */
    v_cs = vert_out.pos;
    v_ndc = vec4_perspective_divide(v_cs);

    /* discard clipping vertices */
    frag_out.clipping = false;
    if (v_ndc.x < -1.0f || v_ndc.x > 1.0f || v_ndc.y < -1.0f || v_ndc.y > 1.0f) {
        frag_out.clipping = true;
    }
    if (hgl_rita_ctx__.opts.z_clipping_enabled && (v_ndc.z < -1.0f || v_ndc.z > 1.0f)) {
        frag_out.clipping = true;
    }

    /* NDC space -> screen space */
    v_ndc.w = 1.0f;
    v_ss = mat4_mul_vec4(hgl_rita_ctx__.tform.viewport, v_ndc);

    /* populate fragment */
#ifndef HGL_RITA_SIMPLE
    frag_out.world_pos     = mat4_mul_vec4(hgl_rita_ctx__.tform.model, v_ls).xyz; // TODO v_ls here is a 'lil sus...
    frag_out.world_tangent = vert_out.tangent;
#endif
    frag_out.world_normal  = vert_out.normal;
    frag_out.uv            = vert_out.uv;
    frag_out.color         = vert_out.color;
    frag_out.x             = v_ss.x;
    frag_out.y             = v_ss.y;
    frag_out.inv_z         = 1.0f / v_ndc.z; // <-- N.B.

    return frag_out;
}

static inline void hgl_rita_process_fragment_internal_(HglRitaFragment *in)
{
    int x = in->x;
    int y = in->y;
    float depth = clamp(0, 1, 1.0f / in->inv_z);
    int s = hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->stride;
    int idx = y * s + x;

#if 0
    /* framebuffer bounds test (shouldn't be necessary anymore) */
    if ((x < 0 || x >= w) ||
        (y < 0 || y >= h)) {
        return;
    }
#endif

    /* framebuffer depth test */
    if ((hgl_rita_ctx__.opts.depth_test_enabled)) {
        assert(hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER] != NULL &&
               "Missing depth attachment in framebuffer (Note: Needed by HGL_RITA_DEPTH_TESTING)");
        if (hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER]->data.r32[idx] < depth) {
            return;
        }
    }

    HglRitaColor color;
    if (hgl_rita_ctx__.shaders.frag == NULL) {
        /* do default shading */
        if (hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DIFFUSE] != NULL) {
            color = hgl_rita_color_mul(in->color, hgl_rita_sample_unit_uv(HGL_RITA_TEX_DIFFUSE, in->uv));
        } else {
            color = in->color;
        }
    } else {
        color = hgl_rita_ctx__.shaders.frag(&hgl_rita_ctx__, in);
    }

    /* alpha blending */
    if (hgl_rita_ctx__.opts.order_dependent_alpha_blending_enabled) {
        float a = (float)color.a / 256.0f;
        color = hgl_rita_color_lerp(hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->data.rgba8[idx], color, a);
        color.a = 255;
    }

    hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_FRAME_BUFFER]->data.rgba8[idx] = color;
    if (hgl_rita_ctx__.opts.depth_buffer_writing_enabled) {
        assert(hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER] != NULL &&
               "Missing depth attachment in framebuffer (Note: Needed by HGL_RITA_DEPTH_BUFFER_WRITING)");
        hgl_rita_ctx__.tex_unit[HGL_RITA_TEX_DEPTH_BUFFER]->data.r32[idx] = depth;
    }
}

static inline HglRitaFragment hgl_rita_frag_lerp_internal_(int x, int y, HglRitaFragment f0, HglRitaFragment f1, float t)
{
    return (HglRitaFragment) {
#ifndef HGL_RITA_SIMPLE
        .world_pos = vec3_lerp(f0.world_pos, f1.world_pos, t),
        .world_tangent = vec3_lerp(f0.world_tangent, f1.world_tangent, t),
#endif
        .world_normal = vec3_lerp(f0.world_normal, f1.world_normal, t),
        .uv = vec2_lerp(f0.uv, f1.uv, t),
        .x = x,
        .y = y,
        .inv_z = lerp(f0.inv_z, f1.inv_z, t),
        .color = hgl_rita_color_lerp(f0.color, f1.color, t),
        .clipping = false, // TODO remove
    };
}

static inline HglRitaFragment hgl_rita_frag_berp_internal_(HglRitaFragment f0,
                                                           HglRitaFragment f1,
                                                           HglRitaFragment f2,
                                                           float u, float v,
                                                           int x, int y)
{
    float w = 1.0f - u - v;
    HglRitaFragment f;

#ifndef HGL_RITA_SIMPLE
    f.world_pos.x = u*f0.world_pos.x + v*f1.world_pos.x + w*f2.world_pos.x;
    f.world_pos.y = u*f0.world_pos.y + v*f1.world_pos.y + w*f2.world_pos.y;
    f.world_pos.z = u*f0.world_pos.z + v*f1.world_pos.z + w*f2.world_pos.z;
    f.world_tangent.x = u*f0.world_tangent.x + v*f1.world_tangent.x + w*f2.world_tangent.x;
    f.world_tangent.y = u*f0.world_tangent.y + v*f1.world_tangent.y + w*f2.world_tangent.y;
    f.world_tangent.z = u*f0.world_tangent.z + v*f1.world_tangent.z + w*f2.world_tangent.z;
#endif
    f.world_normal.x = u*f0.world_normal.x + v*f1.world_normal.x + w*f2.world_normal.x;
    f.world_normal.y = u*f0.world_normal.y + v*f1.world_normal.y + w*f2.world_normal.y;
    f.world_normal.z = u*f0.world_normal.z + v*f1.world_normal.z + w*f2.world_normal.z;
    f.uv.x = u*f0.uv.x + v*f1.uv.x + w*f2.uv.x;
    f.uv.y = u*f0.uv.y + v*f1.uv.y + w*f2.uv.y;
    f.color.r = u*f0.color.r + v*f1.color.r + w*f2.color.r;
    f.color.g = u*f0.color.g + v*f1.color.g + w*f2.color.g;
    f.color.b = u*f0.color.b + v*f1.color.b + w*f2.color.b;
    f.color.a = u*f0.color.a + v*f1.color.a + w*f2.color.a;
    f.x = x;
    f.y = y;
    f.inv_z = u*f0.inv_z + v*f1.inv_z + w*f2.inv_z;
    f.clipping = false;

    return f;
}

static inline float hgl_rita_det_internal_(int f0_x, int f0_y,
                                           int f1_x, int f1_y,
                                           int f2_x, int f2_y)
{
    return (f1_y - f0_y) * (f2_x - f0_x) - (f1_x - f0_x) * (f2_y - f0_y);
}

static inline int hgl_rita_next_vbuf_index_internal_(void)
{
    switch (hgl_rita_ctx__.vertices.mode) {
        case HGL_RITA_ARRAY: {
            if (hgl_rita_ctx__.vertices.counter < hgl_rita_ctx__.vertices.vbuf->length) {
                hgl_rita_ctx__.vertices.counter++;
                return hgl_rita_ctx__.vertices.counter - 1;
            } else {
                return -1;
            }
        } break;
        case HGL_RITA_INDEXED: {
            if (hgl_rita_ctx__.vertices.counter < hgl_rita_ctx__.vertices.ibuf->length) {
                hgl_rita_ctx__.vertices.counter++;
                return hgl_rita_ctx__.vertices.ibuf->arr[hgl_rita_ctx__.vertices.counter - 1];
            } else {
                return -1;
            }
        } break;
        default: assert(0 && "not reachable"); return -1;
    }
}

#endif /* HGL_RITA_IMPLEMENTATION */

// TODO Documentation
// TODO parallelize draw_text?
// TODO HglRitaColor rgba8/r32 union?
// TODO wireframes as primitives?
// TODO cleanup & api redesign
// TODO think some more about the depth buffer...
// https://www.kth.se/social/files/55145c2ff2765464dd7ea176/DH2323%20lab3.pdf
