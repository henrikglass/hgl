
#include "hgl_test.h"

#undef HGL_FLAGS_IMPLEMENTATION /* NOTE: hgl_test.h depends on hgl_flags.h, we don't need
                                         the implementation. */
#include "hgl_flags.h"

TEST(test_flags1)
{
    char *argv[] = {"test_flags1", "-a", "--bee", "-d"};
    int argc = 4;
    bool *opt_a = hgl_flags_add_bool("-a,--aye", "description...", false, 0);
    bool *opt_b = hgl_flags_add_bool("-b,--bee", "description...", false, 0);
    bool *opt_c = hgl_flags_add_bool("-c,--cee", "description...", false, 0);
    bool *opt_d = hgl_flags_add_bool("-d,--dee", "description...", false, 0);
    ASSERT(0 == hgl_flags_parse(argc, argv));
    ASSERT(*opt_a);
    ASSERT(*opt_b);
    ASSERT(!*opt_c);
    ASSERT(*opt_d);
}

TEST(test_flags2, .expect_output = "Hejsan hoppsan\n")
{
    char *argv[] = {"test_flags1", "--aye", "Hejsan hoppsan"};
    int argc = 3;
    const char **opt_a = hgl_flags_add_str("-a,--aye", "description...", false, 0);
    ASSERT(0 == hgl_flags_parse(argc, argv));
    printf("%s\n", *opt_a);
}

TEST(test_occurred_functions)
{
    char *argv[] = {"test_flags1", "-a", "--bee", "-d"};
    int argc = 4;
    bool *opt_a = hgl_flags_add_bool("-a,--aye", "description...", false, 0);
    bool *opt_b = hgl_flags_add_bool("-b,--bee", "description...", false, 0);
    bool *opt_c = hgl_flags_add_bool("-c,--cee", "description...", false, 0);
    bool *opt_d = hgl_flags_add_bool("-d,--dee", "description...", false, 0);
    ASSERT(0 == hgl_flags_parse(argc, argv));
    ASSERT(hgl_flags_occurred_in_args(opt_a));
    ASSERT(hgl_flags_occurred_in_args(opt_b));
    ASSERT(!hgl_flags_occurred_in_args(opt_c));
    ASSERT(hgl_flags_occurred_in_args(opt_d));

    ASSERT(hgl_flags_occurred_before(opt_a, opt_b));
    ASSERT(!hgl_flags_occurred_before(opt_b, opt_a));
    ASSERT(hgl_flags_occurred_before(opt_c, opt_d));
}
