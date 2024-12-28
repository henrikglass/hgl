#include <stdio.h>

#include "hgl_q.h"

int main(void)
{
    HglQueue(int, 4) q = {0};

    int dummy;

    hgl_q_push_back(&q, 1);
    hgl_q_push_back(&q, 2);
    hgl_q_push_back(&q, 3);
    hgl_q_push_back(&q, 4);
    hgl_q_push_back(&q, 5);
    hgl_q_print(&q);
    hgl_q_pop_front(&q, &dummy);
    hgl_q_push_back(&q, 6);
    hgl_q_push_back(&q, 7);
    hgl_q_pop_front(&q, &dummy);
    hgl_q_push_back(&q, 8);
    hgl_q_print(&q);

    return 0;
}
