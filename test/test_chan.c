#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define HGL_CHAN_TYPE int
#define HGL_CHAN_TYPE_ID int
#include "hgl_chan.h"

#define HGL_BUFCHAN_TYPE float
#define HGL_BUFCHAN_TYPE_ID float
#include "hgl_bufchan.h"

#define STR(x)   #x
#define SHOW_DEFINE(x) printf("%s=%s\n", #x, STR(x))

int main(void)
{
    printf("hello\n");

    hgl_int_chan_t c;
    hgl_int_chan_init(&c);
    printf("a\n");
    fflush(stdout);
    hgl_int_chan_send_value(&c, 123);
    printf("b\n");
    fflush(stdout);
    int i = hgl_int_chan_recv(&c);
    (void) i;
    hgl_int_chan_send_value(&c, 10);
    printf("c\n");
    fflush(stdout);
    hgl_int_chan_free(&c);

    hgl_float_bufchan_t bc;
    hgl_float_bufchan_init(&bc, 8);
    hgl_float_bufchan_send_value(&bc, 1.232f);
    float f1 = 69.0f;
    hgl_float_bufchan_send(&bc, &f1);
    hgl_float_bufchan_send_value(&bc, 2.1f);
    hgl_float_bufchan_send_value(&bc, 4.0f);
    hgl_float_bufchan_send_value(&bc, 8.0f);
    hgl_float_bufchan_send_value(&bc, 17.2f);
    float f = hgl_float_bufchan_recv(&bc);
    f = hgl_float_bufchan_recv(&bc);
    f = hgl_float_bufchan_recv(&bc);
    (void) f;
    hgl_float_bufchan_free(&bc);

}
