#include <stdio.h>

typedef void (*proc_t)(const char *args);

#define HGL_CMD_PRIVATE_DATA_T proc_t
#define HGL_CMD_IMPLEMENTATION
#include "hgl_cmd.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void open_door(const char *args);
void open_door(const char *args)
{
    (void) args;
    printf("Opening door\n");
}

void operate_tractor(const char *args);
void operate_tractor(const char *args)
{
    (void) args;
    printf("Operating tractor\n");
}

void operate_bike(const char *args);
void operate_bike(const char *args)
{
    (void) args;
    printf("Operating bike. Pling pling!\n");
}

void my_printf(const char *args);
void my_printf(const char *args)
{
    if (args != NULL)
        printf("%s\n", args);
    else
        printf("is null\n");
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static HglCommand command_tree[] =
{
    {HGL_CMD_LEAF, "hello", "prints \"Hello World!\""},
    {HGL_CMD_LEAF, "goodbye", "prints \"Goodbye folks!\""},
    {HGL_CMD_LEAF, "hejsan", "prints \"Hejsan Hoppsan!\""},
    {HGL_CMD_NODE, "operate", "operates something", .sub_tree = (HglCommand[]){
        {HGL_CMD_LEAF, "forklift", "a forklift"},
        {HGL_CMD_NODE, "vehicle", "some vehicle", .sub_tree = (HglCommand[]){
            {HGL_CMD_LEAF, "car", "a car"},
            {HGL_CMD_LEAF, "bike", "a bike"},
            {HGL_CMD_LEAF, "tractor", "a tractor", .private_data = operate_tractor},
            {HGL_CMD_NONE}
        }},
        {HGL_CMD_NONE}
    }},
    {HGL_CMD_NODE, "aaaa", "", .sub_tree = (HglCommand[]){
        {HGL_CMD_LEAF, "bbbb", "", .private_data = operate_tractor},
        {HGL_CMD_NONE}
    }},
    {HGL_CMD_NODE, "aaaa_", "", .sub_tree = (HglCommand[]){
        {HGL_CMD_LEAF, "bbbb_", "", .private_data = operate_tractor},
        {HGL_CMD_NONE}
    }},
    {HGL_CMD_LEAF, "print", "prints the `args`", .private_data = my_printf},
    {HGL_CMD_NODE, "open", "opens something", .sub_tree = (HglCommand[]){
        {HGL_CMD_LEAF, "jar", "a jar"},
        {HGL_CMD_LEAF, "can", "a can"},
        {HGL_CMD_LEAF, "door", "a door", .private_data = open_door},
        {HGL_CMD_NONE}
    }},
    {HGL_CMD_LEAF, "files", "custom completion example"},
    {HGL_CMD_LEAF, "help", "prints help message"},
    {HGL_CMD_NONE}
};
#pragma GCC diagnostic pop

static bool has_suffix(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    return 0 == memcmp(suffix, str + str_len - suffix_len, suffix_len);
}

static bool is_c_file(const char *str)
{
    return has_suffix(str, ".c") ||
           has_suffix(str, ".h");
}

int main(void)
{
    HglCmdComplBuffer c_source_completer = hgl_cmd_cb_make_from_filesystem(".", 5, -1, is_c_file);
    hgl_cmd_tree_at(command_tree, "files")->compl_buffer = &c_source_completer;
    hgl_cmd_tree_at(command_tree, "operate", "vehicle", "bike")->private_data = operate_bike;

    while (true) {
        const char *args = NULL;
        const HglCommand *cmd = hgl_cmd_input(command_tree, ">>> ", &args);

        if (cmd == NULL) {
            continue;
        }

        if (cmd->kind == HGL_CMD_LEAF && cmd->private_data != NULL) {
            proc_t f = cmd->private_data;
            f(args);
        }

        if (cmd == hgl_cmd_tree_at(command_tree, "help")) {
            const char *end;
            const HglCommand *argscmd = hgl_cmd_tree_at_cstr(command_tree, args, &end);
            if (argscmd != NULL) {
                printf("%s - %s\n", argscmd->name, argscmd->desc);
                if (argscmd->kind == HGL_CMD_NODE) {
                    hgl_cmd_tree_print(argscmd->sub_tree, 2, 42);
                }
            } else {
                hgl_cmd_tree_print(command_tree, 2, 42);
            }
        }
    }
}
