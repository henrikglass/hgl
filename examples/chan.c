#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define HGL_CHAN_IMPLEMENTATION
#include "hgl_chan.h"

#define HGL_BUFFERED_CHAN_IMPLEMENTATION
#include "hgl_buffered_chan.h"

int main(void)
{
    const char *str = "hello world!\n";
    int i0 = 1;
    int i1 = 2;
    int i2 = 3;
    int i3 = 4;
    int i4 = 5;

    HglChan c = hgl_chan_make();
    hgl_chan_send(&c, (void *) str);
    printf("%s", (const char *) hgl_chan_recv(&c));
    hgl_chan_destroy(&c);

    HglBufferedChan bc = hgl_buffered_chan_make(16);
    hgl_buffered_chan_send(&bc, (void *) &i0);
    hgl_buffered_chan_send(&bc, (void *) &i1);
    hgl_buffered_chan_send(&bc, (void *) &i2);
    hgl_buffered_chan_send(&bc, (void *) &i3);
    hgl_buffered_chan_send(&bc, (void *) &i4);

    while (true) {
        void *recv = hgl_buffered_chan_try_recv(&bc);
        if (recv == NULL) break;
        printf("%d\n", *(int *)recv);
    }

}
