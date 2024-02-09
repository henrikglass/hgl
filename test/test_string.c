#include <stdio.h>
#include <assert.h>

#define HGL_MEMDBG_IMPLEMENTATION
#include "hgl_memdbg.h"

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"

int main(void)
{

#if 0
    HglStringBuilder str = hgl_string_builder_make("    hello \n  ", 0);
    HglStringBuilder str2 = hgl_string_builder_make(" hejsan hoppsan    \n    ", 0);
    printf("str1: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    printf("str2: \"%s\" -- len: %zu cap: %zu\n", str2.cstr, str2.length, str2.capacity);
    hgl_string_builder_trim(&str);
    hgl_string_builder_trim(&str2);
    hgl_string_builder_append(&str, str2.cstr, str2.length);
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);

    hgl_string_builder_append_cstr(&str, " world");
    hgl_string_builder_append_cstr(&str, " :-)");
    hgl_string_builder_append_cstr(&str, "a");
    hgl_string_builder_append_cstr(&str, "");
    hgl_string_builder_append_cstr(&str, "23  - ");
    hgl_string_builder_append_cstr(&str, "23  - ");
    hgl_string_builder_append_cstr(&str, "23  - ");
    hgl_string_builder_append_cstr(&str, "23  - ");
    hgl_string_builder_append_fmt(&str, "[%2.2f %4.2f]", 12.34f, 56.78f);
    hgl_string_builder_append_fmt(&str, "fisk");
    hgl_string_builder_shrink_to_fit(&str);
    hgl_string_builder_append(&str, str2.cstr, str2.length);
    
    hgl_string_builder_destroy(&str2);

    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    hgl_string_builder_grow(&str, 10);
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    hgl_string_builder_grow(&str, 32);
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    hgl_string_builder_shrink_to_fit(&str);
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    

    printf("SPLIT ' ': \n");
    HglStringView strv = hgl_string_view_from_sb(&str);
    hgl_string_view_op_begin(&strv);
    int i = 0;
    while (1) {
        i++;
        HglStringView sv = hgl_string_view_split_next(&strv, ' ');
        if (sv.start == NULL) break;
        printf("%d ---- \"%.*s\"\n", i, (int) sv.length, sv.start);
    };
    printf("------------------------\n");

    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    
    printf("FIND L:\n");
    hgl_string_view_op_begin(&strv);
    i = 0;
    while (1) {
        i++;
        HglStringView sv = hgl_string_view_find_next(&strv, "l");
        if ((sv.start == NULL) || i > 15) break;
        printf("%ld\n", sv.start - str.cstr);
        printf("%d ---- \"%.*s\"\n", i, (int) sv.length, sv.start);

    };
    printf("------------------------\n");
    printf("orig: \"%s\" -- len: %zu cap: %zu\n", str.cstr, str.length, str.capacity);
    
    hgl_string_view_op_begin(&strv);
    HglStringView sv = hgl_string_view_find_next(&strv, "fillur");
    printf("asdf%d ---- \"%.*s\"\n", i, (int) sv.length, sv.start);
    assert(sv.length == 0 && sv.start == NULL);


    HglStringView sv2 = hgl_string_view_from_cstr("Fisk fågel strykjärn");
    hgl_string_view_op_begin(&sv2);
    while (1) {
        HglStringView sv = hgl_string_view_split_next(&sv2, ' ');
        if (sv.start == NULL) break;
        printf("%d ---- \"%.*s\"\n", i, (int) sv.length, sv.start);
    };


    printf("%d\n", hgl_string_view_starts_with(&sv2, "Fisk"));
    printf("%d\n", hgl_string_view_contains(&sv2, "fågel "));
    printf("%d\n", hgl_string_view_ends_with(&sv2, "strykjärn"));

    HglStringBuilder sb13 = hgl_string_builder_make(" a      bb ccc dddd  eeeee", 0);
    printf("copy: \"%s\"\n", sb13.cstr);
    hgl_string_builder_replace(&sb13, " ", "----");
    printf("copy: \"%s\"\n", sb13.cstr);

    hgl_string_builder_destroy(&sb13);
    hgl_string_builder_destroy(&str);
    
    HglStringBuilder hsb = hgl_string_builder_make("Hejsan hoppsan fisk fågel!", 0);
    hgl_string_builder_grow(&hsb, 1000);
    hgl_string_builder_replace(&hsb, "fisk", "tjoppsan");
    printf("hsb: \"%s\"\n", hsb.cstr);
    hgl_string_builder_destroy(&hsb);
    
    hsb = hgl_string_builder_make(" aaa bb cc  dddd  ee  ", 256);
    sv = hgl_string_view_from_sb(&hsb);
    hgl_string_view_op_begin(&sv);
    while (1) {
        HglStringView match = hgl_string_view_split_next(&sv, ' ');
        if (match.start == NULL) break;
        printf("\"%.*s\"\n", (int) match.length, match.start); 
    }
    hgl_string_builder_destroy(&hsb);

    printf("\n\n\n\nREGEX:\n");
    sv = hgl_string_view_from_cstr("aaabb23ccccbbsdbbbb");
    hgl_string_view_op_begin(&sv);
    HglStringView match = hgl_string_view_find_next_regex_match(&sv, "[0-9]+");
    match = hgl_string_view_find_next_regex_match(&sv, "b+");
    match = hgl_string_view_find_next_regex_match(&sv, "b+");
    (void) match;

    printf("\n\n\n\nREAD FILE:\n");
    HglStringBuilder sb14 = hgl_string_builder_make("text: ", 0);
    hgl_string_builder_append_file(&sb14, "test/data/text.txt");
    hgl_string_builder_append_file(&sb14, "test/data/text.txt");
    hgl_string_builder_append_file(&sb14, "test/data/text.txt");
    hgl_string_builder_append_file(&sb14, "test/data/text.txt");
    hgl_string_builder_append_file(&sb14, "test/data/text.txt");
    hgl_string_builder_replace(&sb14, ",", "");
    hgl_string_builder_replace_regex(&sb14, "( |\n)", "-");
    printf("\"%s\"\n", sb14.cstr);
    hgl_string_builder_destroy(&sb14);
    
    //printf("\n\n\n\nREGEX REPLACE:\n");
    HglStringBuilder sb15 = hgl_string_builder_make("glass glassigt strutsigt mystigt glassigt\n", 0);
    hgl_string_builder_replace_regex(&sb15, "glass(igt)?", "-");
    hgl_string_builder_replace_regex(&sb15, "\n", "newline");
    //printf("\"%s\"\n", sb15.cstr);

    hgl_string_builder_destroy(&sb15);

#else
    /* example from header library. */
    HglStringBuilder sb = hgl_string_builder_make("", 0);
    hgl_string_builder_append_file(&sb, "test/data/glassigt_lyrics.txt");
    HglStringView sv = hgl_string_view_from_sb(&sb);

    int line_nr = 1;
    hgl_string_view_op_begin(&sv);
    HglStringView line = hgl_string_view_split_next(&sv, '\n');
    while (line.start != NULL) {
        int occurances = 0;
        hgl_string_view_op_begin(&line);
        while (hgl_string_view_find_next(&line, "glassigt").start != NULL) {
            occurances++;
        }
        printf("line #%d has %d occurances of glassigt.\n", line_nr++, occurances);
        line = hgl_string_view_split_next(&sv, '\n');
    }

    hgl_string_builder_destroy(&sb);
#endif


    hgl_memdbg_report();

}
