//#include <stdio.h>

#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"

int main(void)
{
    HglIni ini = hgl_ini_parse("test.ini");

    printf("my_f64 got:        %f\n", hgl_ini_get_f64(&ini, "Things", "my_f64"));
    printf("my_i64 got:        %ld\n", hgl_ini_get_i64(&ini, "Things", "my_i64"));
    printf("my_u64 got:        0x%lX\n", hgl_ini_get_u64(&ini, "Things", "my_u64"));
    printf("my_bool got:       %d\n", hgl_ini_get_bool(&ini, "Things", "my_bool"));
    printf("my_other_bool got: %d\n", hgl_ini_get_bool(&ini, "Things", "my_other_bool"));
    printf("my_other_bool got: %s\n", hgl_ini_get(&ini, "Things", "my_other_bool"));

    printf("###################################\n");
    hgl_ini_print(&ini);
    printf("###################################\n");

    hgl_ini_put(&ini, "Things", "added_bool", "true");
    hgl_ini_put(&ini, "Things", "added_bool", "false");
    hgl_ini_put(&ini, "Things2", "added_thing", "127.0.0.1");
    hgl_ini_put(&ini, "Things2", "added_thing_again", "127.0.0.1");

    printf("###################################\n");
    hgl_ini_print(&ini);
    printf("###################################\n");

    hgl_ini_free(&ini);
    return 0;
}
