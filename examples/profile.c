
#define _GNU_SOURCE

//#define HGL_PROFILE_DETAILED
#define HGL_PROFILE_IMPLEMENTATION
#include "hgl_profile.h"

#include <unistd.h>
#include <stdio.h>

int arr[20000000];

int dummy()
{
    int max = -1000;
    for (int i = 0; i < 20000000; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

int main()
{
    printf("Hello World!\n");
    arr[1000] = 23;

    int max;
    HGL_PROFILE_MEASURE(max = dummy());
    printf("max = %d\n", max);
    hgl_profile_report(0);
    hgl_profile_reset();

    hgl_profile_begin(__FILE__);
    for (int i = 0; i < 10; i++) {
        hgl_profile_begin("A/1");
        usleep(10000);
        hgl_profile_end();
        hgl_profile_begin("A/2");
        hgl_profile_end();
        hgl_profile_begin("A/3");
        usleep(2000);
        hgl_profile_end();
        hgl_profile_report(HGL_PROFILE_TIME_LAST);
    }
    hgl_profile_end();

    hgl_profile_report(HGL_PROFILE_EVERYTHING);
}
