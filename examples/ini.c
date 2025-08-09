//#include <stdio.h>

//#define HGL_MEMDBG_IMPLEMENTATION
//#include "hgl_memdbg.h"

#define HGL_ALLOC_IMPLEMENTATION
#include "hgl_alloc.h"


static HglAllocator my_allocator;

static inline void *my_alloc(size_t size) { return hgl_alloc(&my_allocator, size); }
static inline void *my_realloc(void *ptr, size_t size) { return hgl_realloc(&my_allocator, ptr, size); }
static inline void my_free(void *ptr) { hgl_free(&my_allocator, ptr); }
#define HGL_INI_ALLOC my_alloc
#define HGL_INI_REALLOC my_realloc
#define HGL_INI_FREE my_free
#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"


int main(void)
{

    my_allocator = hgl_alloc_make(.kind = HGL_FREE_STACK_ALLOCATOR, 
                                  .size = 2*1024, 
                                  .free_stack_capacity = 4);

    HglIni *ini = hgl_ini_open("assets/test.ini");
    if (ini == NULL) {
        return 1;
    }

    printf("my_f64 got:        %f\n", hgl_ini_get_f64(ini, "Things", "my_f64"));
    printf("my_i64 got:        %ld\n", hgl_ini_get_i64(ini, "Things", "my_i64"));
    printf("my_u64 got:        0x%lX\n", hgl_ini_get_u64(ini, "Things", "my_u64"));
    printf("my_bool got:       %d\n", hgl_ini_get_bool(ini, "Things", "my_bool"));
    printf("my_other_bool got: %d\n", hgl_ini_get_bool(ini, "Things", "my_other_bool"));
    printf("my_other_bool got: %s\n", hgl_ini_get(ini, "Things", "my_other_bool"));

    printf("###################################\n");
    hgl_ini_fprint(stdout, ini);
    printf("###################################\n");

    hgl_ini_put(ini, "Things", "added_bool", "true");
    hgl_ini_put(ini, "Things", "added_bool", "false");
    hgl_ini_put(ini, "Things2", "added_thing", "127.0.0.1");
    hgl_ini_put(ini, "Things2", "added_thing_again", "127.0.0.1");

    printf("###################################\n");
    hgl_ini_fprint(stdout, ini);
    printf("###################################\n");

    //hgl_ini_free(ini);
    //
    hgl_free_all(&my_allocator);

#if 0
    ini = hgl_ini_create("testingtesting.ini");
    hgl_ini_put(ini, "Things", "added_bool", "true");
    hgl_ini_put(ini, "Things", "added_bool", "false");
    hgl_ini_put(ini, "Things2", "added_thing", "127.0.0.1");
    hgl_ini_put(ini, "Things2", "added_thing_again", "127.0.0.1");
    hgl_ini_save(ini, NULL);
#endif

    //hgl_memdbg_report();
    return 0;
}
