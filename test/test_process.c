
#include "hgl_test.h"

typedef struct {
    int i;
    const char *str;
} MyData;

#define HGL_CMD_IMPLEMENTATION
#define HGL_CMD_PRIVATE_DATA_T MyData

#include "hgl_cmd.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static const HglCommand cmd_tree[] =
{
    {HGL_CMD_NODE, "say", "say something", .sub_tree = (HglCommand[]) {
        {HGL_CMD_LEAF, "hello", "prints \"Hello World!\""},
        {HGL_CMD_LEAF, "goodbye", "prints \"Goodbye folks!\""},
        {HGL_CMD_NONE}
    }},
    {HGL_CMD_NODE, "data", "description", .sub_tree = (HglCommand[]) {
        {HGL_CMD_LEAF, "a", "description", .private_data = {.i = 6, .str = "A"}},
        {HGL_CMD_LEAF, "b", "description", .private_data = {.i = 7, .str = "B"}},
        {HGL_CMD_LEAF, "c", "description", .private_data = {.i = 8, .str = "C"}},
        {HGL_CMD_NONE}
    }},
    {HGL_CMD_NONE}
};
#pragma GCC diagnostic pop

TEST(test_say_hello, .input = "say hello 123\n")
{
    const char *args;
    const HglCommand *cmd = hgl_cmd_input(cmd_tree, ">>> ", &args);
    ASSERT(cmd == hgl_cmd_tree_at(cmd_tree, "say", "hello"));
    ASSERT_CSTR_EQ(args, "123");
}

TEST(test_get_data, .input = "data b\n")
{
    const char *args;
    const HglCommand *cmd = hgl_cmd_input(cmd_tree, ">>> ", &args);
    ASSERT(cmd == hgl_cmd_tree_at(cmd_tree, "data", "b"));
    ASSERT(cmd->private_data.i == 7);
    ASSERT_CSTR_EQ(cmd->private_data.str, "B");
}
