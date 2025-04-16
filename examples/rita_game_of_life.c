#define _DEFAULT_SOURCE

#define HGL_RITA_IMPLEMENTATION
#include "hgl_rita.h"

#include "raylib.h"

#define WIDTH          1024
#define HEIGHT         1024
#define DISPLAY_SCALE   1.0

static bool paused = false;
static bool fancy = true;
static HglRitaTexture *src = NULL;
static HglRitaTexture *dst = NULL;

static inline HglRitaColor game_of_life_iteration(const HglRitaContext *ctx, const HglRitaFragment *in)
{
    (void) ctx;

    HglRitaColor alive_color = HGL_RITA_WHITE;
    HglRitaColor dead_color = HGL_RITA_BLACK;
    if (fancy) {
        dead_color = in->color;
        dead_color.r *= 0.99f;
        dead_color.g *= 0.96f;
        dead_color.b *= 0.90f;
    }

    bool is_alive = (in->color.r == 255);

    int n_live_neighbours = 0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            if (x == 0 && y == 0) continue;
            if (hgl_rita_sample(src, in->x + x, in->y + y).r == 255) {
                n_live_neighbours++;
            }
        }
    }

    if (paused) {
        return (is_alive) ? alive_color : dead_color;
    }

    if (is_alive) {
        if (n_live_neighbours < 2) return dead_color;
        if (n_live_neighbours < 4) return alive_color;
        return dead_color;
    } else {
        if (n_live_neighbours == 3) return alive_color;
        return dead_color;
    }

}

void randomize_grid(HglRitaTexture *tex)
{
    uint32_t n = tex->stride * tex->height;
    for (uint32_t i = 0; i < n; i++) {
        if (rand() & 1) {
            tex->data.rgba8[i] = HGL_RITA_BLACK;
        } else {
            tex->data.rgba8[i] = HGL_RITA_WHITE;
        }
    }
}

int main()
{
    /* Initialize hgl_rita */
    hgl_rita_init();
    hgl_rita_use_clear_color(HGL_RITA_BLACK);

    /* Create a framebuffer texture and bind it (depth buffer is optional) */
    HglRitaTexture fbs[2] = {
        hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8),
        hgl_rita_texture_make(WIDTH, HEIGHT, HGL_RITA_RGBA8),
    };

    /* psuedo-randomize initial state */
    srand(1337);
    randomize_grid(&fbs[0]);

    /* Setup hgl_rita so we can draw stuff using screen coordinates */
    Mat4 view = mat4_make_ortho(0, DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, 0, -10, 10);
    hgl_rita_use_view_matrix(view);
    hgl_rita_use_viewport(WIDTH, HEIGHT);
    HglRitaVertexBuffer vbuf = {0};
    hgl_rita_bind_buffer(HGL_RITA_VERTEX_BUFFER, &vbuf);
    hgl_rita_disable(HGL_RITA_DEPTH_TESTING |
                     HGL_RITA_DEPTH_BUFFER_WRITING);

    /* Raylib stuff: IGNORE */
    InitWindow(DISPLAY_SCALE*WIDTH, DISPLAY_SCALE*HEIGHT, "HglRita: Game of Life!");
    Image color_image = (Image) {
        .data    = fbs[0].data.rgba8,
        .width   = WIDTH,
        .height  = HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture2D color_tex = LoadTextureFromImage(color_image);

    int frame_count = 0;
    SetTargetFPS(60);
    Vector2 last_mouse_pos = GetMousePosition();
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q))
    {
        src = &fbs[(frame_count) % 2];
        dst = &fbs[(frame_count + 1) % 2];

        /* Draw! */
        hgl_rita_bind_texture(HGL_RITA_TEX_FRAME_BUFFER, dst);
        hgl_rita_blit(0, 0, WIDTH, HEIGHT, src,
                      HGL_RITA_REPLACE,
                      HGL_RITA_EVERYWHERE,
                      HGL_RITA_SHADER,
                      game_of_life_iteration);
        hgl_rita_finish();

        /* options */
        if (IsKeyPressed(KEY_F)) fancy = !fancy;
        if (IsKeyPressed(KEY_P)) paused = !paused;
        if (IsKeyPressed(KEY_R)) randomize_grid(dst);
        if (IsKeyPressed(KEY_B)) hgl_rita_clear(HGL_RITA_COLOR);

        /* Drawing */
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            last_mouse_pos = GetMousePosition();
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_pos = GetMousePosition();
            hgl_rita_buf_push(&vbuf, (HglRitaVertex){
                .pos.x = last_mouse_pos.x,
                .pos.y = last_mouse_pos.y,
                .color = HGL_RITA_WHITE
            });
            hgl_rita_buf_push(&vbuf, (HglRitaVertex){
                .pos.x = mouse_pos.x,
                .pos.y = mouse_pos.y,
                .color = HGL_RITA_WHITE
            });
            if (mouse_pos.x == last_mouse_pos.x && mouse_pos.y == last_mouse_pos.y) {
                hgl_rita_draw(HGL_RITA_POINTS);
            } else {
                hgl_rita_draw(HGL_RITA_LINES);
            }
            hgl_rita_finish();
            hgl_rita_buf_clear(&vbuf);
            last_mouse_pos = mouse_pos;
        }

        /* raylib stuff: IGNORE */
        UpdateTexture(color_tex, dst->data.rgba8);
        BeginDrawing();
            DrawTextureEx(color_tex, (Vector2){0, 0}, 0, DISPLAY_SCALE, WHITE);
        EndDrawing();

        frame_count++;
    }

    CloseWindow();
}
