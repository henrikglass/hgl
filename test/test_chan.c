#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define HGL_CHAN_TYPE int
#define HGL_CHAN_TYPE_ID int
#include "hgl_chan.h"

#define HGL_BUFFERED_CHAN_TYPE float
#define HGL_BUFFERED_CHAN_TYPE_ID float
#include "hgl_buffered_chan.h"

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
    
    // select test
    hgl_int_chan_t c1;
    hgl_int_chan_init(&c1);
    hgl_int_chan_t c2;
    hgl_int_chan_init(&c2);
    hgl_int_chan_t c3;
    hgl_int_chan_init(&c3);
    printf("&c1 is %p\n", (void *) &c1);
    printf("&c2 is %p\n", (void *) &c2);
    printf("&c3 is %p\n", (void *) &c3);
    hgl_int_chan_send_value(&c1, 123);
    hgl_int_chan_send_value(&c2, 456);
    hgl_int_chan_t *ret = hgl_int_chan_select(3, &c1, &c2, &c3);
    int val = hgl_int_chan_recv(ret);
    if (ret == &c1) {
        printf("c1!\n");
    } else if (ret == &c2) {
        printf("c2!\n");
    } else if (ret == &c3) {
        printf("c3!\n");
    }

    printf("returned1: %p\n", (void *) ret);
    printf("read value1: %d\n", val);
    ret = hgl_int_chan_select(3, &c1, &c2, &c3);
    val = hgl_int_chan_recv(ret);
    printf("returned2: %p\n", (void *) ret);
    printf("read value2: %d\n", val);
    hgl_int_chan_free(&c1);
    hgl_int_chan_free(&c2);
    hgl_int_chan_free(&c3);


    hgl_float_buffered_chan_t bc1;
    hgl_float_buffered_chan_t bc2;
    hgl_float_buffered_chan_init(&bc1, 8);
    hgl_float_buffered_chan_init(&bc2, 8);
    hgl_float_buffered_chan_send_value(&bc1, 1.232f);
    float f1 = 69.0f;
    hgl_float_buffered_chan_send(&bc1, &f1);
    hgl_float_buffered_chan_send_value(&bc1, 2.1f);
    hgl_float_buffered_chan_send_value(&bc1, 4.0f);
    hgl_float_buffered_chan_send_value(&bc1, 8.0f);
    hgl_float_buffered_chan_send_value(&bc2, 1000.0f);
    hgl_float_buffered_chan_send_value(&bc2, 1100.0f);
    hgl_float_buffered_chan_send_value(&bc2, 1200.0f);
    hgl_float_buffered_chan_send_value(&bc1, 17.2f);

    while (true) {
        hgl_float_buffered_chan_t *retf = hgl_float_buffered_chan_select(2, &bc1, &bc2);
        if (retf == &bc1) {
            printf("receive from bc1: ");
        } else if (retf == &bc2) {
            printf("receive from bc2: ");
        } 
        float f = hgl_float_buffered_chan_recv(retf);
        printf("%f\n", f);
    }

    //float f = hgl_float_buffered_chan_recv(&bc1);
    //f = hgl_float_buffered_chan_recv(&bc1);
    //f = hgl_float_buffered_chan_recv(&bc1);
    //(void) f;
    //hgl_float_buffered_chan_free(&bc1);

}
