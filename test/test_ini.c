
#include "hgl_test.h"

#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"

static HglIni *ini;

GLOBAL_SETUP {
    ini = hgl_ini_parse("data/test.ini");
}

GLOBAL_TEARDOWN {
    hgl_ini_free(ini);
}

TEST(test_read) {
    uint64_t my_u64 = hgl_ini_get_u64(ini, "Things", "my_u64");
    ASSERT(my_u64 == 0xDEADBEEF);
}

TEST(test_put) {
    hgl_ini_put(ini, "MySecion", "Hello", "hi");
    hgl_ini_put(ini, "MyEntirelyNewSecion", "Goodbye", "bye");
    ASSERT(0xDEADBEEF == hgl_ini_get_u64(ini, "Things", "my_u64"));
    ASSERT_CSTR_EQ(hgl_ini_get(ini, "MySecion", "Hello"), "hi");
    ASSERT_CSTR_EQ(hgl_ini_get(ini, "MyEntirelyNewSecion", "Goodbye"), "bye");
}

TEST(test_print, .expect_output = 
        "[Things]\n"
        "my_f64 = 123.456\n"
        "my_i64 = 1337\n"
        "my_u64 = 0xDEADBEEF\n"
        "my_bool = True\n"
        "my_other_bool = True\n"
        "\n"
        "[MySection]\n"
        "\n"
        "[MyOtherSection]\n"
        "kr i nk o = pl i nko\n"
        "\n"
        "[MySectionAtEnd]\n"
        "\n"
        "[MySecion]\n"
        "Hello = hi\n"
        "\n"
        "[MyEntirelyNewSecion]\n"
        "Goodbye = bye\n"
        "\n") {
    hgl_ini_put(ini, "MySecion", "Hello", "hi");
    hgl_ini_put(ini, "MyEntirelyNewSecion", "Goodbye", "bye");
    hgl_ini_fprint(stdout, ini);
}

TEST(test_print2) {
    char buf[4096];
    memset(buf, '\0', 4096);
    hgl_ini_put(ini, "MySecion", "Hello", "hi");
    hgl_ini_put(ini, "MyEntirelyNewSecion", "Goodbye", "bye");
    FILE *stream = fmemopen(buf, 4096 - 1, "wb");
    hgl_ini_fprint(stream, ini);
    ASSERT_CSTR_EQ(FILE_CONTENTS("data/test.ini.with_changes"), buf);
}
