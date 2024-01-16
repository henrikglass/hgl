#define HGL_HOTLOAD_DEBUG_PRINTS
#define HGL_HOTLOAD_IMPLEMENTATION
#include "hgl_hotload.h"

#include <stdio.h>

#include <time.h>

typedef void (*mylib_print_t)(void);
static mylib_print_t mylib_print = NULL;

void hotload_setup(void);
void hotload_setup()
{
    hgl_hotload_init(HGL_HOTLOAD_LIBS("libmylib.so"), RTLD_NOW);
    hgl_hotload_add_symbol((void *) &mylib_print, "libmylib.so", "mylib_print"); 
    hgl_hotload_force_reload_all(); 
    assert(mylib_print != NULL);
}

int main(void)
{
    hotload_setup();

    struct timespec ts = {
        .tv_sec  = 1,
        .tv_nsec = 0
    };
    struct timespec ts2 = {0};

    for (int i = 0; i < 100; i++) {
        nanosleep(&ts, &ts2);
        printf("Hello from main program! %d \n", i);
        mylib_print();
        assert(0 == hgl_hotload_reload_lib("libmylib.so")); 
    }
    
    hgl_hotload_final();
}
