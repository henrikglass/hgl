
#include "hgl_test.h"

#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"

static HglIni *ini;

GLOBAL_SETUP
{
    ini = hgl_ini_open("assets/test.ini");
}

GLOBAL_TEARDOWN
{
    hgl_ini_close(ini);
}

TEST(test_read)
{
    uint64_t my_u64 = hgl_ini_get_u64(ini, "Things", "my_u64");
    ASSERT(my_u64 == 0xDEADBEEF);
}

TEST(test_has)
{
    ASSERT(hgl_ini_has(ini, "Things", "my_u64"));
    ASSERT(!hgl_ini_has(ini, "Kalas", "my_u64"));
    ASSERT(!hgl_ini_has(ini, "Things", "struts"));
}

TEST(test_put)
{
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
        "\n")
{
    hgl_ini_put(ini, "MySecion", "Hello", "hi");
    hgl_ini_put(ini, "MyEntirelyNewSecion", "Goodbye", "bye");
    hgl_ini_fprint(stdout, ini);
}

TEST(test_print2) 
{
    char buf[4096];
    memset(buf, '\0', 4096);
    hgl_ini_put(ini, "MySecion", "Hello", "hi");
    hgl_ini_put(ini, "MyEntirelyNewSecion", "Goodbye", "bye");
    FILE *stream = fmemopen(buf, 4096 - 1, "wb");
    hgl_ini_fprint(stream, ini);
    ASSERT_CSTR_EQ(FILE_CONTENTS("assets/test.ini.with_changes"), buf);
}

TEST(
    test_print_empty, 
    .expect_output = ""
) {
    HglIni *ini2 = hgl_ini_create("test");
    hgl_ini_fprint(stdout, ini2);
}

TEST(
    test_print_empty_2, 
    .expect_output = 
        "[MySection]\n"
        "MyKey = MyValue\n\n"
) {
    HglIni *ini2 = hgl_ini_create("test");
    hgl_ini_put(ini2, "MySection", "MyKey", "MyValue");
    hgl_ini_fprint(stdout, ini2);
}

TEST(test_has_section)
{
    ASSERT(hgl_ini_has_section(ini, "MySection"));
    ASSERT(hgl_ini_has_section(ini, "MyOtherSection"));
    ASSERT(!hgl_ini_has_section(ini, "Kraxflax"));
}

TEST(
    test_iterate, 
    .timeout = 1,
    .expect_output = 
        "S0\n"
        "k0=v0\n"
        "k1=v1\n"
        "k2=v2\n"
        "S1\n"
        "k0=v0\n"
        "k1=v1\n"
        "k2=v2\n"
        "S2\n"
        "k0=v0\n"
) {
    HglIni *ini2 = hgl_ini_create("test");

    hgl_ini_put(ini2, "S0", "k0", "v0");
    hgl_ini_put(ini2, "S0", "k1", "v1");
    hgl_ini_put(ini2, "S0", "k2", "v2");
    hgl_ini_put(ini2, "S1", "k0", "v0");
    hgl_ini_put(ini2, "S1", "k1", "v1");
    hgl_ini_put(ini2, "S1", "k2", "v2");
    hgl_ini_put(ini2, "S2", "k0", "v0");

    hgl_ini_reset_section_iterator(ini2);
    HglIniSection *s = hgl_ini_next_section(ini2);
    while (s != NULL) {
        printf("%s\n", s->name);
        hgl_ini_reset_kv_pair_iterator(s);
        HglIniKVPair *kv = hgl_ini_next_kv_pair(s);
        while (kv != NULL) {
            printf("%s=%s\n", kv->key, kv->val);
            kv = hgl_ini_next_kv_pair(s);
        }
        s = hgl_ini_next_section(ini2);
    }
}
