
#include "hgl_test.h"

#define HGL_CHAN_IMPLEMENTATION
#include "hgl_chan.h"

static HglChan c;

GLOBAL_SETUP {
    c = hgl_chan_make();
}

GLOBAL_TEARDOWN {
    hgl_chan_destroy(&c);
}

TEST(test_simple) {
    int i = 42;
    hgl_chan_send(&c, (void *) &i); 
    ASSERT(42 == *(int *)hgl_chan_recv(&c));
}

TEST(test_should_block_send) {
    int i = 42;
    hgl_chan_send(&c, (void *) &i); 
    ASSERT(-1 == hgl_chan_try_send(&c, (void *) &i));
}

TEST(test_should_block_recv) {
    ASSERT(NULL == hgl_chan_try_recv(&c));
}

TEST(test_select) {
    int i = 42;

    HglChan c0 = hgl_chan_make();
    HglChan c1 = hgl_chan_make();
    HglChan c2 = hgl_chan_make();
    HglChan c3 = hgl_chan_make();

    hgl_chan_send(&c2, (void *) &i);
    hgl_chan_send(&c3, (void *) &i);

    HglChan *selected = hgl_chan_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c2);
    selected = hgl_chan_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c2);
    hgl_chan_recv(&c2);

    selected = hgl_chan_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c3);
    hgl_chan_recv(&c3);

    selected = hgl_chan_try_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == NULL);

    hgl_chan_send(&c2, (void *) &i);

    selected = hgl_chan_try_select(4, &c0, &c1, &c2, &c3);
    ASSERT(selected == &c2);
}

