
#define HGL_PROGRESS_BAR_IMPLEMENTATION
#include "hgl_progress_bar.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>

static void sleep_ms(double ms);
static void sleep_ms(double ms)
{
    uint64_t sec = (uint64_t)(ms / 1000.0);
    uint64_t nsec = ((uint64_t)(ms * 1000000.0)) % 1000000000lu;
    struct timespec ts = {.tv_sec = sec, .tv_nsec = nsec};
    while (-1 == nanosleep(&ts, &ts));
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int N = 1000;

    hgl_progress_bar_use_style(HGL_PROGRESS_BAR_ASCII);
    hgl_progress_bar_begin("progress:");
    for (int i = 0; i <= N; i++) {
        sleep_ms(1.0);
        float t = (float)i/(float)N;
        hgl_progress_bar_update(t);
    }
    hgl_progress_bar_end();

    hgl_progress_bar_use_style(HGL_PROGRESS_BAR_BLOCK);
    hgl_progress_bar_begin("progress:");
    for (int i = 0; i <= N; i++) {
        sleep_ms(1.0);
        float t = (float)i/(float)N;
        hgl_progress_bar_update(t);
    }
    hgl_progress_bar_end();

    hgl_progress_bar_use_style(HGL_PROGRESS_BAR_LINE);
    hgl_progress_bar_begin("progress:");
    for (int i = 0; i <= N; i++) {
        sleep_ms(1.0);
        float t = (float)i/(float)N;
        hgl_progress_bar_update(t);
    }
    hgl_progress_bar_end();

}
