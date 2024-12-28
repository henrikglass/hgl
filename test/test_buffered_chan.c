
#include "hgl_test.h"

#define HGL_BUFFERED_CHAN_IMPLEMENTATION
#include "hgl_buffered_chan.h"

static HglBufferedChan c;

GLOBAL_SETUP {
    c = hgl_buffered_chan_make(4);
}

GLOBAL_TEARDOWN {
    hgl_buffered_chan_destroy(&c);
}

TEST(test_simple) {
    int i = 42;
    hgl_buffered_chan_send(&c, (void *) &i); 
    ASSERT(42 == *(int *)hgl_buffered_chan_recv(&c));
}

TEST(test_should_block_send) {
    int i = 42;
    hgl_buffered_chan_send(&c, (void *) &i); 
    hgl_buffered_chan_send(&c, (void *) &i); 
    hgl_buffered_chan_send(&c, (void *) &i); 
    ASSERT(-1 == hgl_buffered_chan_try_send(&c, (void *) &i));
}

TEST(test_should_block_recv) {
    ASSERT(NULL == hgl_buffered_chan_try_recv(&c));
}

TEST(test_should_block_recv2) {
    int i = 42;
    int i2 = 64;
    hgl_buffered_chan_send(&c, (void *) &i); 
    hgl_buffered_chan_send(&c, (void *) &i2); 
    ASSERT((void *) &i == hgl_buffered_chan_recv(&c));
    ASSERT((void *) &i2 == hgl_buffered_chan_recv(&c));
    ASSERT(NULL == hgl_buffered_chan_try_recv(&c));
}

TEST(test_select) {
    int i = 42;

    HglBufferedChan c0 = hgl_buffered_chan_make(4);
    HglBufferedChan c1 = hgl_buffered_chan_make(4);
    HglBufferedChan c2 = hgl_buffered_chan_make(4);
    HglBufferedChan c3 = hgl_buffered_chan_make(4);

    hgl_buffered_chan_send(&c2, (void *) &i);
    hgl_buffered_chan_send(&c2, (void *) &i); // send twice
    hgl_buffered_chan_send(&c3, (void *) &i);

    HglBufferedChan *selected = hgl_buffered_chan_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c2);
    selected = hgl_buffered_chan_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c2);
    hgl_buffered_chan_recv(&c2);
    selected = hgl_buffered_chan_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c2);
    hgl_buffered_chan_recv(&c2);

    selected = hgl_buffered_chan_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c3);
    hgl_buffered_chan_recv(&c3);

    selected = hgl_buffered_chan_try_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == NULL);

    hgl_buffered_chan_send(&c2, (void *) &i);

    selected = hgl_buffered_chan_try_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c2);
}

