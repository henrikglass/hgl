#include <stdio.h>
#include <assert.h>

#define HGL_MEMDBG_IMPLEMENTATION
#include "hgl_memdbg.h"

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"

int main(void)
{

#if 0
    HglStringBuilder str = hgl_sb_make("    hello \n  ", 0);
    HglStringBuilder str2 = hgl_sb_make(" hejsan hoppsan    \n    ", 0);
    printf("str1: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    printf("str2: \"%s\" -- len: %zu cap: %zu\n", str2.cstr, str2.length, str2.capacity);
    hgl_sb_trim(&str);
    hgl_sb_trim(&str2);
    hgl_sb_append(&str, str2.cstr, str2.length);
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);

    hgl_sb_append_cstr(&str, " world");
    hgl_sb_append_cstr(&str, " :-)");
    hgl_sb_append_cstr(&str, "a");
    hgl_sb_append_cstr(&str, "");
    hgl_sb_append_cstr(&str, "23  - ");
    hgl_sb_append_cstr(&str, "23  - ");
    hgl_sb_append_cstr(&str, "23  - ");
    hgl_sb_append_cstr(&str, "23  - ");
    hgl_sb_append_fmt(&str, "[%2.2f %4.2f]", 12.34f, 56.78f);
    hgl_sb_append_fmt(&str, "fisk");
    hgl_sb_shrink_to_fit(&str);
    hgl_sb_append(&str, str2.cstr, str2.length);
    
    hgl_sb_destroy(&str2);

    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    hgl_sb_grow(&str, 10);
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    hgl_sb_grow(&str, 32);
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    hgl_sb_shrink_to_fit(&str);
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    

    printf("SPLIT ' ': \n");
    HglStringView strv = hgl_sv_from_sb(&str);
    hgl_sv_op_begin(&strv);
    int i = 0;
    while (1) {
        i++;
        HglStringView sv = hgl_sv_split_next(&strv, ' ');
        if (sv.start == NULL) break;
        printf("%d ---- \"%.*s\"\n", i, (int) sv.length, sv.start);
    };
    printf("------------------------\n");

    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    
    printf("FIND L:\n");
    hgl_sv_op_begin(&strv);
    i = 0;
    while (1) {
        i++;
        HglStringView sv = hgl_sv_find_next(&strv, "l");
        if ((sv.start == NULL) || i > 15) break;
        printf("%ld\n", sv.start - str.cstr);
        printf("%d ---- \"%.*s\"\n", i, (int) sv.length, sv.start);

    };
    printf("------------------------\n");
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    
    hgl_sv_op_begin(&strv);
    HglStringView sv = hgl_sv_find_next(&strv, "fillur");
    printf("asdf%d ---- \"%.*s\"\n", i, (int) sv.length, sv.start);
    assert(sv.length == 0 && sv.start == NULL);


    HglStringView sv2 = hgl_sv_from_cstr("Fisk fågel strykjärn");
    hgl_sv_op_begin(&sv2);
    while (1) {
        HglStringView sv = hgl_sv_split_next(&sv2, ' ');
        if (sv.start == NULL) break;
        printf("%d ---- \"%.*s\"\n", i, (int) sv.length, sv.start);
    };


    printf("%d\n", hgl_sv_starts_with(&sv2, "Fisk"));
    printf("%d\n", hgl_sv_contains(&sv2, "fågel "));
    printf("%d\n", hgl_sv_ends_with(&sv2, "strykjärn"));

    HglStringBuilder sb13 = hgl_sb_make(" a      bb ccc dddd  eeeee", 0);
    printf("copy: \"%s\"\n", sb13.cstr);
    hgl_sb_replace(&sb13, " ", "----");
    printf("copy: \"%s\"\n", sb13.cstr);

    hgl_sb_destroy(&sb13);
    hgl_sb_destroy(&str);
    
    HglStringBuilder hsb = hgl_sb_make("Hejsan hoppsan fisk fågel!", 0);
    hgl_sb_grow(&hsb, 1000);
    hgl_sb_replace(&hsb, "fisk", "tjoppsan");
    printf("hsb: \"%s\"\n", hsb.cstr);
    hgl_sb_destroy(&hsb);
    
    hsb = hgl_sb_make(" aaa bb cc  dddd  ee  ", 256);
    sv = hgl_sv_from_sb(&hsb);
    hgl_sv_op_begin(&sv);
    while (1) {
        HglStringView match = hgl_sv_split_next(&sv, ' ');
        if (match.start == NULL) break;
        printf("\"%.*s\"\n", (int) match.length, match.start); 
    }
    hgl_sb_destroy(&hsb);

    printf("\n\n\n\nREGEX:\n");
    sv = hgl_sv_from_cstr("aaabb23ccccbbsdbbbb");
    hgl_sv_op_begin(&sv);
    HglStringView match = hgl_sv_find_next_regex_match(&sv, "[0-9]+");
    match = hgl_sv_find_next_regex_match(&sv, "b+");
    match = hgl_sv_find_next_regex_match(&sv, "b+");
    (void) match;

    printf("\n\n\n\nREAD FILE:\n");
    HglStringBuilder sb14 = hgl_sb_make("text: ", 0);
    hgl_sb_append_file(&sb14, "assets/text.txt");
    hgl_sb_append_file(&sb14, "assets/text.txt");
    hgl_sb_append_file(&sb14, "assets/text.txt");
    hgl_sb_append_file(&sb14, "assets/text.txt");
    hgl_sb_append_file(&sb14, "assets/text.txt");
    hgl_sb_replace(&sb14, ",", "");
    hgl_sb_replace_regex(&sb14, "( |\n)", "-");
    printf("\"%s\"\n", sb14.cstr);
    hgl_sb_destroy(&sb14);
    
    //printf("\n\n\n\nREGEX REPLACE:\n");
    HglStringBuilder sb15 = hgl_sb_make("glass glassigt strutsigt mystigt glassigt\n", 0);
    hgl_sb_replace_regex(&sb15, "glass(igt)?", "-");
    hgl_sb_replace_regex(&sb15, "\n", "newline");
    //printf("\"%s\"\n", sb15.cstr);

    hgl_sb_destroy(&sb15);

#endif
#if 0
    /* example from header library. */
    HglStringBuilder sb = hgl_sb_make("", 0);
    hgl_sb_append_file(&sb, "assets/glassigt_lyrics.txt");
    HglStringView sv = hgl_sv_from_sb(&sb);

    int line_nr = 1;
    hgl_sv_op_begin(&sv);
    HglStringView line = hgl_sv_split_next(&sv, '\n');
    while (line.start != NULL) {
        int occurances = 0;
        hgl_sv_op_begin(&line);
        while (hgl_sv_find_next(&line, "glassigt").start != NULL) {
            occurances++;
        }
        printf("line #%d has %d occurances of glassigt.\n", line_nr++, occurances);
        line = hgl_sv_split_next(&sv, '\n');
    }

    hgl_sb_destroy(&sb);
#endif
#if 0
    HglStringView sv = hgl_sv_from_cstr("bingo/bango/bongo - bish/bash/bosh");
    printf(HGL_SV_FMT "\n", HGL_SV_ARG(sv));
    HglStringView left_of_minus = hgl_sv_lchop_until(&sv, '-');
    printf(HGL_SV_FMT "|" HGL_SV_FMT "\n", HGL_SV_ARG(left_of_minus), HGL_SV_ARG(sv));

    HglStringView bingo = hgl_sv_lchop_until(&left_of_minus, '/');
    HglStringView bango = hgl_sv_lchop_until(&left_of_minus, '/');
    HglStringView bongo = hgl_sv_trim(hgl_sv_lchop_until(&left_of_minus, '/'));

    HglStringView bosh = hgl_sv_rchop_until(&sv, '/');
    HglStringView bash = hgl_sv_rchop_until(&sv, '/');
    HglStringView bish = hgl_sv_rchop_until(&sv, '/');

    printf("bingo? " HGL_SV_FMT "!\n", HGL_SV_ARG(bingo));
    printf("bango? " HGL_SV_FMT "!\n", HGL_SV_ARG(bango));
    printf("bongo? " HGL_SV_FMT "!\n", HGL_SV_ARG(bongo));

    printf("bish? " HGL_SV_FMT "!\n", HGL_SV_ARG(bish));
    printf("bash? " HGL_SV_FMT "!\n", HGL_SV_ARG(bash));
    printf("bosh? " HGL_SV_FMT "!\n", HGL_SV_ARG(bosh));

    printf("sv length = %zu\n", sv.length);
    printf("left_of_minus length = %zu\n", left_of_minus.length);

#endif
#if 1
    printf(HGL_SV_FMT "\n", HGL_SV_ARG(HGL_SV("hello!")));

    HglStringView csv = HGL_SV("1234, 4.123 , 0x1337, 420, 69, 123, hej");
    int ival = (int) hgl_sv_lchop_i64(&csv);
    hgl_sv_lchop_until(&csv, ',');
    float fval = (float) hgl_sv_lchop_f64(&csv);
    hgl_sv_lchop_until(&csv, ',');
    unsigned int uval = (unsigned int) hgl_sv_lchop_u64(&csv);

    printf("ival = %d\n", ival);
    printf("fval = %f\n", (double) fval);
    printf("uval = 0x%X\n", uval);

    int should_be_420 = hgl_sv_to_i64(&csv);
    int should_be_420_again = hgl_sv_to_i64(&csv);

    assert(should_be_420 == should_be_420_again);
    printf("Yay!\n");

    /* should put us at the end of the string. I.e. csv->length should be 0 */
    HglStringView left;
    printf(HGL_SV_FMT "\n", HGL_SV_ARG(csv));
    left = hgl_sv_lchop_until(&csv, ',');

    printf("--------\n");
    left = hgl_sv_lchop_until(&csv, ',');
    printf(HGL_SV_FMT "\n", HGL_SV_ARG(left));
    printf("--------\n");
    left = hgl_sv_lchop_until(&csv, ',');
    printf(HGL_SV_FMT "\n", HGL_SV_ARG(left));
    printf("--------\n");
    left = hgl_sv_lchop_until(&csv, ',');
    printf(HGL_SV_FMT "\n", HGL_SV_ARG(left));
    printf("--------\n");
    left = hgl_sv_lchop_until(&csv, ',');
    printf(HGL_SV_FMT "\n", HGL_SV_ARG(left));
    printf("csv->length = %zu\n", csv.length);

    HglStringView greeting = HGL_SV("Hello friend!");
    HglStringView hello = hgl_sv_lchop(&greeting, 5);
    HglStringView friend = hgl_sv_ltrim(greeting);
    printf(HGL_SV_FMT " " HGL_SV_FMT "\n", HGL_SV_ARG(hello), HGL_SV_ARG(friend));

    HglStringBuilder sb = hgl_sb_make("", 0);
    hgl_sb_append_sv(&sb, &hello);
    hgl_sb_append_char(&sb, ',');
    hgl_sb_append_char(&sb, ' ');
    hgl_sb_append_sv(&sb, &friend);
    printf("sb: \"%s\"\n", sb.cstr);
    hgl_sb_destroy(&sb);

    HglStringView a = HGL_SV("Hello!");
    HglStringView b = HGL_SV("Hello!");
    assert(hgl_sv_equals(a, b));

#endif


    hgl_memdbg_report();

}
