#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define HGL_RING_BUFFER_POW2_CAPACITY
#define HGL_RING_BUFFER_TYPE float
#define HGL_RING_BUFFER_TYPE_ID float
#include "hgl_ring_buffer.h"

#define STR(x)   #x
#define SHOW_DEFINE(x) printf("%s=%s\n", #x, STR(x))

int main(void)
{

    hgl_float_ring_buffer_t rbuf;
    
    float *f;
    float f1 = 12345.6f;
    hgl_float_ring_buffer_init(&rbuf, 8);
    hgl_float_ring_buffer_push_back_value(&rbuf, 1.0f);
    hgl_float_ring_buffer_push_back(&rbuf, &f1);
    hgl_float_ring_buffer_push_back_value(&rbuf, 3.0f);
    printf("Hello!\n");
    hgl_float_ring_buffer_push_back_value(&rbuf, 6.0f);
    f = hgl_float_ring_buffer_pop_front(&rbuf);
    f = hgl_float_ring_buffer_pop_front(&rbuf);
    f = hgl_float_ring_buffer_pop_front(&rbuf);
    f = hgl_float_ring_buffer_pop_front(&rbuf);
    f = hgl_float_ring_buffer_pop_front(&rbuf);
    f = hgl_float_ring_buffer_pop_front(&rbuf);
    hgl_float_ring_buffer_push_back_value(&rbuf, 1.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 1.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 1.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 2.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 3.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 6.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 2.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 3.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 6.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 2.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 3.0f);
    hgl_float_ring_buffer_push_back_value(&rbuf, 6.0f);
    f = hgl_float_ring_buffer_pop_front(&rbuf);
    hgl_float_ring_buffer_free(&rbuf);
    (void) f;

}
