
#include "hgl_test.h"

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"

static HglStringBuilder sb;
static HglStringView sv;

GLOBAL_SETUP {
    sb = hgl_sb_make(.initial_capacity = 4096);
    hgl_sb_append_cstr(&sb, "Hejsan hoppsan! \n Hello World!\n 0x000A, 20, 40, 80");
    sv = hgl_sv_from_sb(&sb);
}

GLOBAL_TEARDOWN {
    hgl_sb_destroy(&sb);
}

TEST(sv_lchop1, .expect_output = " Hello World!") {
    HglStringView row0 = hgl_sv_lchop_until(&sv, '\n');
    HglStringView row1 = hgl_sv_lchop_until(&sv, '\n');
    HglStringView row2 = hgl_sv_lchop_until(&sv, '\n');

    printf(HGL_SV_FMT, HGL_SV_ARG(row1));
}

TEST(sv_lchop2, .expect_output = " 0x000A, 20, 40, 80") {
    HglStringView row0 = hgl_sv_lchop_until(&sv, '\n');
    HglStringView row1 = hgl_sv_lchop_until(&sv, '\n');
    HglStringView row2 = hgl_sv_lchop_until(&sv, '\n');

    printf(HGL_SV_FMT, HGL_SV_ARG(row2));
    ASSERT(sv.length == 0);
}

TEST(sv_lchop_int) {
    hgl_sv_lchop_until(&sv, '\n');
    hgl_sv_lchop_until(&sv, '\n');
    HglStringView row2 = hgl_sv_lchop_until(&sv, '\n');

    int sum = 0;
    sum += hgl_sv_lchop_i64(&row2); hgl_sv_lchop_until(&row2, ',');
    sum += hgl_sv_lchop_i64(&row2); hgl_sv_lchop_until(&row2, ',');
    sum += hgl_sv_lchop_i64(&row2); hgl_sv_lchop_until(&row2, ',');
    sum += hgl_sv_lchop_i64(&row2); hgl_sv_lchop_until(&row2, ',');
    ASSERT(sum == 150);
}

TEST(sv_ltrim, .expect_output = "0x000A, 20, 40, 80") {
    HglStringView row0 = hgl_sv_lchop_until(&sv, '\n');
    HglStringView row1 = hgl_sv_lchop_until(&sv, '\n');
    HglStringView row2 = hgl_sv_lchop_until(&sv, '\n');
    row2 = hgl_sv_ltrim(row2);

    printf(HGL_SV_FMT, HGL_SV_ARG(row2));
    ASSERT(sv.length == 0);
}

TEST(sv_rtrim1, .expect_output = "Hejsan hoppsan!") {
    HglStringView row0 = hgl_sv_lchop_until(&sv, '\n');
    row0 = hgl_sv_rtrim(row0);

    printf(HGL_SV_FMT, HGL_SV_ARG(row0));
}

TEST(sv_rtrim2, .expect_output = " 0x000A, 20, 40, 80") {
    HglStringView row0 = hgl_sv_lchop_until(&sv, '\n');
    HglStringView row1 = hgl_sv_lchop_until(&sv, '\n');
    HglStringView row2 = hgl_sv_lchop_until(&sv, '\n');
    row2 = hgl_sv_rtrim(row2);

    printf(HGL_SV_FMT, HGL_SV_ARG(row2));
    ASSERT(sv.length == 0);
}

TEST(sv_contains) {
    ASSERT(hgl_sv_contains(&sv, "Hejsan"));
    ASSERT(hgl_sv_contains(&sv, "hoppsan"));
    ASSERT(hgl_sv_contains(&sv, "!"));
    ASSERT(!hgl_sv_contains(&sv, "struts"));
}

TEST(sv_compare) {
    ASSERT(0 == hgl_sv_compare(HGL_SV_LIT("Hejsan"), HGL_SV_LIT("Hejsan")));
    ASSERT(-1 == hgl_sv_compare(HGL_SV_LIT("AAAA"), HGL_SV_LIT("AAAB")));
    ASSERT(1 == hgl_sv_compare(HGL_SV_LIT("BAAA"), HGL_SV_LIT("AAAB")));
}

TEST(sb_search_and_replace, .expect_output = "Bejsan boppsan! \n Bello World!\n 0x000A, 20, 40, 80") {
    hgl_sb_replace(&sb, "He", "Be"); 
    hgl_sb_replace(&sb, "ho", "bo"); 
    printf(HGL_SV_FMT, HGL_SV_ARG(sv)); // string view is a view into the string 
                                        // owned by sb in this case.
}

TEST(sb_append_self, .expect_output = "Hejsan hoppsan! \n Hello World!\n 0x000A, 20, 40, 80"
                                      "Hejsan hoppsan! \n Hello World!\n 0x000A, 20, 40, 80") {
    hgl_sb_append_sv(&sb, &sv); 
    sv = hgl_sv_from_sb(&sb);
    printf(HGL_SV_FMT, HGL_SV_ARG(sv)); // string view is a view into the string 
                                        // owned by sb in this case.
}

TEST(sb_append_file, .expect_output = "fisk\n") {
    hgl_sb_clear(&sb); 
    hgl_sb_append_file(&sb, "assets/fisk.txt");
    sv = hgl_sv_from_sb(&sb);
    printf(HGL_SV_FMT, HGL_SV_ARG(sv)); // string view is a view into the string 
                                        // owned by sb in this case.
    hgl_sb_shrink_to_fit(&sb);
    ASSERT(sb.capacity == strlen("fisk\n") + 1);
}
