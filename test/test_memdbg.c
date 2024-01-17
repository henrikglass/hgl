#include <stdio.h>
#include <stdlib.h>

#define HGL_MEMDGB_IMPLEMENTATION
#include "hgl_memdbg.h"

int main(void)
{

    double *d = malloc(sizeof(double)); 
    double *arr = malloc(64 * sizeof(double)); 
    arr[0] = 1.0f;
    arr[1] = 2.0f;
    arr[2] = 3.0f;
    arr[3] = 4.0f;
    double *arr2 = malloc(64 * sizeof(double)); 
    //(void) arr;
    //(void) arr2;

    *d = 123.456;

    printf("Hello world! %f\n", *d);
    
    //free(arr3);
    //free(d);
    //free(arr2);
    double *arr3 = malloc(64 * sizeof(double)); 
    //(void) arr3;
    free(arr2);
    //free(d);
    printf("%f\n", arr[2]);
    arr = realloc(arr, 128*sizeof(double));
    printf("%f\n", arr[2]);
    //free(arr3);
    free(arr);
    free(d);

    int has_leak = hgl_memdbg_report();
    return (has_leak) ? 1 : 0;
}
