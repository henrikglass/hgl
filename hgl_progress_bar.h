
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2026 Henrik A. Glass
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
 * hgl_progress_bar.h - quick and dirty progress bar.
 *
 *
 * USAGE:
 *
 * See examples/ directory or the example below.
 *
 *
 * EXAMPLE:
 *
 *     #define HGL_PROGRESS_BAR_HIDE_CURSOR                 // (optional) default is to display the cursor
 *     #define HGL_PROGRESS_BAR_REFRESH_PERIOD_MS 50        // (optional) default is 100 ms
 *     #define HGL_PROGRESS_BAR_IMPLEMENTATION
 *     #include "hgl_progress_bar.h"
 *                     
 *                     .
 *                     .
 *                     .
 *    
 *     hgl_progress_bar_use_style(HGL_PROGRESS_BAR_LINE);   // (optional) Coolest style B) (default)
 *     hgl_progress_bar_use_fixed_width(80);                // (optional) Width is fixed to 80 columns
 *     hgl_progress_bar_use_dynamic_width();                // (optional) Width is decided by terminal dimensions (default)
 *     hgl_progress_bar_begin("my progress bar:");
 *     for (int i = 0; i <= N; i++) {
 *         sleep_ms(1.0);
 *         float t = (float)i/(float)N;
 *         hgl_progress_bar_update(t);
 *     }
 *     hgl_progress_bar_end();
 * 
 * 
 * See the examples directory.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_PROGRESS_BAR_H
#define HGL_PROGRESS_BAR_H

/*--- Include files ---------------------------------------------------------------------*/

/*--- Public macros ---------------------------------------------------------------------*/

#ifndef HGL_PROGRESS_BAR_REFRESH_PERIOD_MS
#  define HGL_PROGRESS_BAR_REFRESH_PERIOD_MS 100
#endif

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    HGL_PROGRESS_BAR_ASCII,
    HGL_PROGRESS_BAR_BLOCK,
    HGL_PROGRESS_BAR_LINE,
    HGL_PROGRESS_BAR_BLOCK_PRECISE,
} HglProgressBarStyle;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void hgl_progress_bar_use_style(HglProgressBarStyle style);
void hgl_progress_bar_use_fixed_width(int width);
void hgl_progress_bar_use_dynamic_width(void);
int hgl_progress_bar_get_term_width(void);
void hgl_progress_bar_begin(const char *fmt, ...);
void hgl_progress_bar_end(void);
void hgl_progress_bar_update(float completion);

#endif /* HGL_PROGRESS_BAR_H */

#ifdef HGL_PROGRESS_BAR_IMPLEMENTATION

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <locale.h>
#include <assert.h>
#include <stdarg.h>

#define SBUF_LENGTH (16*1024)
typedef struct
{
    int length;
    char buf[SBUF_LENGTH];
} HglProgressBarStringBuffer;

static inline void hgl_progress_bar_sb_append(HglProgressBarStringBuffer *sb, const char *str);
static inline void hgl_progress_bar_sb_append_sb(HglProgressBarStringBuffer *sb, HglProgressBarStringBuffer *other);
static inline void hgl_progress_bar_sb_append_fmt(HglProgressBarStringBuffer *sb, const char *fmt, ...);
static inline void hgl_progress_bar_sb_append_vfmt(HglProgressBarStringBuffer *sb, const char *fmt, va_list args);
static inline uint64_t hgl_progress_bar_get_timestamp_ms(void);

static struct
{
    HglProgressBarStyle style;
    HglProgressBarStringBuffer text;
    int fixed_width;
    int terminal_width;
    bool use_fixed_width;
    bool force_redraw_once;
    bool is_completed;
} hgl_progress_bar_ctx_ = {
    .style             = HGL_PROGRESS_BAR_LINE,
    .text              = { .buf[0] = '\0', .length = 0 },
    .fixed_width       = 80,
    .terminal_width    = 0,
    .use_fixed_width   = 0,
    .force_redraw_once = 1,
    .is_completed      = 0,
};

void hgl_progress_bar_use_style(HglProgressBarStyle style)
{
    hgl_progress_bar_ctx_.style = style;
}

void hgl_progress_bar_use_fixed_width(int width)
{
    hgl_progress_bar_ctx_.use_fixed_width = true;
    hgl_progress_bar_ctx_.fixed_width = width;
}

void hgl_progress_bar_use_dynamic_width()
{
    hgl_progress_bar_ctx_.use_fixed_width = false;
}

int hgl_progress_bar_get_term_width()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    hgl_progress_bar_ctx_.terminal_width = w.ws_col;
    return hgl_progress_bar_ctx_.terminal_width;
}

void hgl_progress_bar_begin(const char *fmt, ...)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    hgl_progress_bar_ctx_.text.length = 0;
    va_list args;
    va_start(args, fmt);
    hgl_progress_bar_sb_append_vfmt(&hgl_progress_bar_ctx_.text, fmt, args);
    va_end(args);

    if (hgl_progress_bar_ctx_.style != HGL_PROGRESS_BAR_ASCII) {
        char *locale = setlocale(LC_ALL, "");
        if (locale == NULL) {
            hgl_progress_bar_ctx_.style = HGL_PROGRESS_BAR_ASCII;
        }
    }

    hgl_progress_bar_get_term_width();
    int width = 0;
    if (hgl_progress_bar_ctx_.use_fixed_width) {
        width = hgl_progress_bar_ctx_.fixed_width;
    } else {
        width = hgl_progress_bar_ctx_.terminal_width;
    }
    assert(4*width < SBUF_LENGTH); // 4 = max # bytes per character in UTF-8
#ifdef HGL_PROGRESS_BAR_HIDE_CURSOR
    printf("\33[?25l");
#endif
    hgl_progress_bar_ctx_.force_redraw_once = true;
    hgl_progress_bar_update(0.0f);
}

void hgl_progress_bar_end()
{
    setvbuf(stdout, NULL, _IOFBF, 0);
    hgl_progress_bar_ctx_.force_redraw_once = true;
    hgl_progress_bar_ctx_.is_completed = true;
    hgl_progress_bar_update(1.0f);
    printf("\n");
#ifdef HGL_PROGRESS_BAR_HIDE_CURSOR
    printf("\33[?25h");
#endif
}

void hgl_progress_bar_update(float completion)
{
    static uint64_t redraw_count = 0;
    static HglProgressBarStringBuffer sbs[2] = {0};
    static HglProgressBarStringBuffer *sb_current = &sbs[0];
    static HglProgressBarStringBuffer *sb_previous = &sbs[1];
    static uint64_t last_draw_time = 0;

    /* Force redraw? */
    bool force_complete_redraw = hgl_progress_bar_ctx_.force_redraw_once;

    /* Too early to redraw? Skip */
    uint64_t now_ms = hgl_progress_bar_get_timestamp_ms();
    if (!force_complete_redraw && ((now_ms - last_draw_time) < HGL_PROGRESS_BAR_REFRESH_PERIOD_MS)) {
        return;
    }

    /* swap buffers */
    HglProgressBarStringBuffer *tmp = sb_current;
    sb_current = sb_previous;
    sb_previous = tmp;

    /* clear current buffer */
    sb_current->length = 0;

    /* clamp completion to [0, 1] */
    completion = (completion > 1.0f) ? 1.0f :
                 (completion < 0.0f) ? 0.0f :
                  completion;

    /* determine which width value to use */
    int width = 0;
    if (hgl_progress_bar_ctx_.use_fixed_width) {
        width = hgl_progress_bar_ctx_.fixed_width;
    } else {
        width = hgl_progress_bar_ctx_.terminal_width;
    }

    /* draw progress bar */
    switch (hgl_progress_bar_ctx_.style) {
        case HGL_PROGRESS_BAR_ASCII: {
            if (hgl_progress_bar_ctx_.is_completed) {
                hgl_progress_bar_sb_append(sb_current, " . ");
            } else {
                static const char *spinner[4] = {"-", "\\", "|", "/"};
                hgl_progress_bar_sb_append_fmt(sb_current, " %s ", spinner[redraw_count % 4]);
            }
            hgl_progress_bar_sb_append_sb(sb_current, &hgl_progress_bar_ctx_.text);
            int bar_width = width - sb_current->length - 12; 
            int n_complete = (int)(completion * (float)bar_width);
            int n_incomplete = bar_width - n_complete;
            hgl_progress_bar_sb_append(sb_current, " [");
            for (int i = 0; i < n_complete; i++) hgl_progress_bar_sb_append(sb_current, "=");
            for (int i = 0; i < n_incomplete; i++) hgl_progress_bar_sb_append(sb_current, " ");
            hgl_progress_bar_sb_append(sb_current, "]");
            hgl_progress_bar_sb_append_fmt(sb_current, " %5.1f %%", (double)completion * 100.0);
        } break;

        case HGL_PROGRESS_BAR_BLOCK: {
            if (hgl_progress_bar_ctx_.is_completed) {
                hgl_progress_bar_sb_append(sb_current, " \u2713 ");
            } else {
                static const char *spinner[8] = {"\u28F7", "\u28EF", "\u28DF", "\u287F", "\u28BF", "\u28FB", "\u28FD", "\u28FE"};
                hgl_progress_bar_sb_append_fmt(sb_current, " %s ", spinner[redraw_count % 8]);
            }
            hgl_progress_bar_sb_append_sb(sb_current, &hgl_progress_bar_ctx_.text);
            int bar_width = width - sb_current->length - 8; 
            int n_complete = (int)(completion * (float)bar_width);
            int n_incomplete = bar_width - n_complete;
            hgl_progress_bar_sb_append(sb_current, " ");
            for (int i = 0; i < n_complete; i++) hgl_progress_bar_sb_append(sb_current, "\u2588");
            for (int i = 0; i < n_incomplete; i++) hgl_progress_bar_sb_append(sb_current, "\u2591");
            hgl_progress_bar_sb_append_fmt(sb_current, " %5.1f %%", (double)completion * 100.0);
        } break;

        case HGL_PROGRESS_BAR_LINE: {
            if (hgl_progress_bar_ctx_.is_completed) {
                hgl_progress_bar_sb_append(sb_current, " \u2713 ");
            } else {
                static const char *spinner[4] = {"\u280B", "\u2819", "\u281A", "\u2813"};
                hgl_progress_bar_sb_append_fmt(sb_current, " %s ", spinner[redraw_count % 4]);
            }
            hgl_progress_bar_sb_append_sb(sb_current, &hgl_progress_bar_ctx_.text);
            int bar_width = width - sb_current->length - 8; 
            int n_complete = (int)(completion * (float)bar_width);
            int n_incomplete = bar_width - n_complete;
            hgl_progress_bar_sb_append(sb_current, " ");
            for (int i = 0; i < n_complete; i++) hgl_progress_bar_sb_append(sb_current, "\u2500");
            for (int i = 0; i < n_incomplete; i++) hgl_progress_bar_sb_append(sb_current, " ");
            hgl_progress_bar_sb_append_fmt(sb_current, " %5.1f %%", (double)completion * 100.0);
        } break;

        case HGL_PROGRESS_BAR_BLOCK_PRECISE: {
            if (hgl_progress_bar_ctx_.is_completed) {
                hgl_progress_bar_sb_append(sb_current, " \u2713 ");
            } else {
                static const char *spinner[8] = {"\u28F7", "\u28EF", "\u28DF", "\u287F", "\u28BF", "\u28FB", "\u28FD", "\u28FE"};
                hgl_progress_bar_sb_append_fmt(sb_current, " %s ", spinner[redraw_count % 8]);
            }
            hgl_progress_bar_sb_append_sb(sb_current, &hgl_progress_bar_ctx_.text);
            int bar_width = width - sb_current->length - 8; 

            int n_complete = (int)(completion * (float)bar_width);
            int n_incomplete = bar_width - n_complete - 1;
            int fract = 8 * (completion * (float)bar_width - n_complete);
            hgl_progress_bar_sb_append(sb_current, " ");
            for (int i = 0; i < n_complete; i++) hgl_progress_bar_sb_append(sb_current, "\u2588");
            switch (fract) {
                case 0: n_incomplete++; break;
                case 1: hgl_progress_bar_sb_append(sb_current, "\u258F"); break;
                case 2: hgl_progress_bar_sb_append(sb_current, "\u258E"); break;
                case 3: hgl_progress_bar_sb_append(sb_current, "\u258D"); break;
                case 4: hgl_progress_bar_sb_append(sb_current, "\u258C"); break;
                case 5: hgl_progress_bar_sb_append(sb_current, "\u258B"); break;
                case 6: hgl_progress_bar_sb_append(sb_current, "\u258A"); break;
                case 7: hgl_progress_bar_sb_append(sb_current, "\u2589"); break;
            }
            for (int i = 0; i < n_incomplete; i++) hgl_progress_bar_sb_append(sb_current, " ");
            hgl_progress_bar_sb_append_fmt(sb_current, " %5.1f %%", (double)completion * 100.0);
        } break;
    }
    
    redraw_count++;
    last_draw_time = now_ms;

    if (force_complete_redraw) {
        printf("\r%s", sb_current->buf);
        goto out;
    } 

    printf("\r");
    int col = 1;
    for (int i = 0; i < sb_current->length; i++) {
        uint8_t c = (uint8_t)sb_current->buf[i];
        if ((c & 0x80) == 0) {
            if (sb_current->buf[i] != sb_previous->buf[i]) {
                printf("\033[%dG", col);
                fputc(sb_current->buf[i], stdout);
            }
        } else if ((c >> 5) == 0x06) {
            if (sb_current->buf[i + 1] != sb_previous->buf[i + 1]) {
                printf("\033[%dG", col);
                fputc(sb_current->buf[i], stdout);
                fputc(sb_current->buf[i + 1], stdout);
            }
            i += 1;
        } else if ((c >> 4) == 0x0E) {
            if ((sb_current->buf[i + 1] != sb_previous->buf[i + 1]) ||
                (sb_current->buf[i + 2] != sb_previous->buf[i + 2])) {
                printf("\033[%dG", col);
                fputc(sb_current->buf[i], stdout);
                fputc(sb_current->buf[i + 1], stdout);
                fputc(sb_current->buf[i + 2], stdout);
            }
            i += 2;
        } else if ((c >> 3) == 0x1E) {
            if ((sb_current->buf[i + 1] != sb_previous->buf[i + 1]) ||
                (sb_current->buf[i + 2] != sb_previous->buf[i + 2]) ||
                (sb_current->buf[i + 3] != sb_previous->buf[i + 3])) {
                printf("\033[%dG", col);
                fputc(sb_current->buf[i], stdout);
                fputc(sb_current->buf[i + 1], stdout);
                fputc(sb_current->buf[i + 2], stdout);
                fputc(sb_current->buf[i + 3], stdout);
            }
            i += 3;
        }
        col++;
    }

out:
    hgl_progress_bar_ctx_.force_redraw_once = false;
    hgl_progress_bar_ctx_.is_completed = false;
    printf("\33[%dG", width);
    fflush(stdout);
}

/*--- Private functions -----------------------------------------------------------------*/

static inline void hgl_progress_bar_sb_append(HglProgressBarStringBuffer *sb, const char *str)
{
    sb->length += sprintf(&sb->buf[sb->length], "%s", str);
    sb->buf[sb->length] = '\0';
}

static inline void hgl_progress_bar_sb_append_sb(HglProgressBarStringBuffer *sb, HglProgressBarStringBuffer *other)
{
    sb->length += sprintf(&sb->buf[sb->length], "%s", other->buf);
    sb->buf[sb->length] = '\0';
}

static inline void hgl_progress_bar_sb_append_fmt(HglProgressBarStringBuffer *sb, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    sb->length += vsprintf(&sb->buf[sb->length], fmt, args);
    va_end(args);
    sb->buf[sb->length] = '\0';
}

static inline void hgl_progress_bar_sb_append_vfmt(HglProgressBarStringBuffer *sb, const char *fmt, va_list args)
{
    sb->length += vsprintf(&sb->buf[sb->length], fmt, args);
    sb->buf[sb->length] = '\0';
}

static inline uint64_t hgl_progress_bar_get_timestamp_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000lu + ts.tv_nsec / 1000000lu; 
}

#endif /* HGL_PROGRESS_BAR_IMPLEMENTATION */
