
typedef void (*proc_t)(void);

#define HGL_CMD_PRIVATE_DATA_T proc_t
#define HGL_CMD_IMPLEMENTATION
#include "hgl_cmd.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void open_door(void);
void open_door()
{
    printf("Opening door\n");
}

void operate_tractor(void);
void operate_tractor()
{
    printf("Operating tractor\n");
}

void operate_bike(void);
void operate_bike()
{
    printf("Operating bike. Pling pling!\n");
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
    {HGL_CMD_NODE, "open", "opens something", .sub_tree = (HglCommand[]){
        {HGL_CMD_LEAF, "jar", "a jar"},
        {HGL_CMD_LEAF, "can", "a can"},
        {HGL_CMD_LEAF, "door", "a door", .private_data = open_door},
        {HGL_CMD_NONE}
    }},
    {HGL_CMD_LEAF, "help", "prints help message"},
    {HGL_CMD_NONE}
};
#pragma GCC diagnostic pop

int main()
{
    hgl_cmd_tree_at(command_tree, "operate", "vehicle", "bike")->private_data = operate_bike;

    while (true) {
        const char *args;
        const HglCommand *cmd = hgl_cmd_input(command_tree, ">>> ", &args);
        if (cmd->private_data != NULL) {
            proc_t f = cmd->private_data;
            f();
        }
        if (cmd == hgl_cmd_tree_at(command_tree, "help")) {
            hgl_cmd_tree_print(command_tree, 2, 42);
        }
    }
}
