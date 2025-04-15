//#include <stdio.h>

//#define HGL_MEMDBG_IMPLEMENTATION
//#include "hgl_memdbg.h"

#define HGL_FS_ALLOC_IMPLEMENTATION
#include "hgl_fs_alloc.h"


static HglFsAllocator fs_allocator;

void *fs_alloc(size_t size);
void fs_free(void *ptr);
void fs_free_all(void);

void *fs_alloc(size_t size) {
    printf("<ALLOC>\n");
    return hgl_fs_alloc(&fs_allocator, size);
}

void *fs_realloc(void *ptr, size_t size) {
    printf("<REALLOC>\n");
    return hgl_fs_realloc(&fs_allocator, ptr, size);
}

void fs_free(void *ptr) {
    printf("<FREE>\n");
    hgl_fs_free(&fs_allocator, ptr);
}

void fs_free_all(void) {
    hgl_fs_free_all(&fs_allocator);
}

#define HGL_INI_ALLOC fs_alloc
#define HGL_INI_REALLOC fs_realloc
#define HGL_INI_FREE fs_free
#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"


int main(void)
{

    fs_allocator = hgl_fs_make(2*1024, 4);

    HglIni *ini = hgl_ini_parse("assets/test.ini");
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
    fs_free_all();

    printf("{%zu -- %zu}\n", fs_allocator.free_stack[fs_allocator.free_count - 1].start - fs_allocator.memory,
                             fs_allocator.free_stack[fs_allocator.free_count - 1].end - fs_allocator.memory);

    //hgl_memdbg_report();
    return 0;
}
