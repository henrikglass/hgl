
#include "hgl_test.h"

#include "hgl_q.h"

typedef HglQueue(int, 8) IntQueue;

static IntQueue q = {0};

TEST(test_push) 
{
    hgl_q_push_back(&q, 1);
    hgl_q_push_back(&q, 2);
    hgl_q_push_back(&q, 3);
    hgl_q_push_back(&q, 4);
    ASSERT(0 == hgl_q_push_back(&q, 5));
}

TEST(test_capacity) 
{
    hgl_q_push_back(&q, 3);
    hgl_q_push_back(&q, 4);
    ASSERT(8 == hgl_q_capacity(&q));
}

TEST(test_push_too_many) 
{
    ASSERT(!hgl_q_is_full(&q));
    ASSERT(hgl_q_is_empty(&q));
    hgl_q_push_back(&q, 1);
    hgl_q_push_back(&q, 2);
    hgl_q_push_back(&q, 3);
    hgl_q_push_back(&q, 4);
    ASSERT(0 == hgl_q_push_back(&q, 5));
    ASSERT(0 == hgl_q_push_back(&q, 6));
    ASSERT(0 == hgl_q_push_back(&q, 7));
    ASSERT(!hgl_q_is_full(&q));
    ASSERT(!hgl_q_is_empty(&q));
    ASSERT(0 == hgl_q_push_back(&q, 8));
    ASSERT(hgl_q_is_full(&q));
    ASSERT(!hgl_q_is_empty(&q));
    ASSERT(0 != hgl_q_push_back(&q, 9));
}

TEST(test_push_pop) 
{
    hgl_q_push_back(&q, 1);
    hgl_q_push_back(&q, 2);
    hgl_q_push_back(&q, 3);
    hgl_q_push_back(&q, 4);
    int a, b, c, d, e;
    ASSERT(0 == hgl_q_pop_front(&q, &a));
    ASSERT(0 == hgl_q_pop_front(&q, &b));
    ASSERT(0 == hgl_q_pop_front(&q, &c));
    ASSERT(0 == hgl_q_pop_front(&q, &d));
    ASSERT(0 != hgl_q_pop_front(&q, &e));
    ASSERT(a == 1);
    ASSERT(b == 2);
    ASSERT(c == 3);
    ASSERT(d == 4);
    hgl_q_push_back(&q, 5);
    ASSERT(0 == hgl_q_pop_front(&q, &e));
    ASSERT(e == 5);
}
