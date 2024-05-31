
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_cmd.h implements a simple command prompt user interface.
 *
 *
 * USAGE:
 *
 * The user must define their own command tree, as such:
 *
 *     static HglCommand cmd_tree[] =
 *     {
 *         {HGL_CMD_NODE, "say", "say something", .sub_tree = (HglCommand[]) =
 *             {HGL_CMD_LEAF, "hello", "prints \"Hello World!\""},
 *             {HGL_CMD_LEAF, "goodbye", "prints \"Goodbye folks!\""},
 *             {HGL_CMD_NONE}
 *         },
 *         {HGL_CMD_NONE}
 *     };
 *
 * Each command (or node) in the command tree is either of the type `HGL_CMD_NODE`,
 * `HGL_CMD_LEAF`, or `HGL_CMD_NONE`. Each `HGL_CMD_NODE` must define a `sub_tree`
 * with the last child being of type `HGL_CMD_NONE`, which indicates that there
 * are no more children in the sub-tree. Each `HGL_CMD_LEAF` may have some data
 * associated with it in it's `private_data` field. The type of `private_data` is
 * `void *` by default, but may be changed to some other type, for instance a
 * function pointer, by redefining:
 *
 *     HGL_CMD_PRIVATE_DATA_T
 *
 * User input example:
 *
 *     #define BUF_SIZE 64
 *     char buf[BUF_SIZE];
 *     const HglCommand *cmd_hi  = hgl_cmd_tree_at(cmd_tree, "say", "hello");
 *     const HglCommand *cmd_bye = hgl_cmd_tree_at(cmd_tree, "say", "goodbye");
 *     const HglCommand *cmd     = hgl_cmd_input(cmd_tree, "$ ", buf, BUF_SIZE, NULL);
 *     if (cmd == cmd_hi) {
 *         printf("Hello World!\n");
 *     } else if (cmd == cmd_bye) {
 *         printf("Goodbye folks!\n");
 *     }
 *
 * The caller or `hgl_cmd_input` may parse additional data in the supplied input
 * buffer (`buf` in this case) by providing a pointer to size_t as the last argument
 * (lpos). Upon returning, `hgl_cmd_input` will set lpos to point to the first byte
 * inside `buf` which is not part of the string corresponding with a certain path
 * in the command tree (`hgl_cmd_input` returns a pointer to this command).
 *
 * See the test/-directory for a more complete example.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_CMD_H
#define HGL_CMD_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stddef.h>
#include <stdbool.h>

/*--- Public macros ---------------------------------------------------------------------*/

#ifndef HGL_CMD_PRIVATE_DATA_T
#define HGL_CMD_PRIVATE_DATA_T void *
#endif

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct HglCommand HglCommand;

typedef enum
{
    HGL_CMD_NODE,
    HGL_CMD_LEAF,
    HGL_CMD_NONE
} HglCommandKind;

struct HglCommand
{
    HglCommandKind kind;
    const char *name;
    const char *desc;
    union {
        HGL_CMD_PRIVATE_DATA_T private_data;
        const HglCommand *sub_tree;
    };
};

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

/**
 * Prompt the user for input.
 *
 * @param       command_tree    A command tree.
 * @param       prompt          An optional prompt prefix, e.g. ">>> " or "$ ".
 * @param       buf             A buffer to store the input in. Must be zeroed.
 * @param       buf_size        The size of `buf`.
 * @param[out]  lpos            The last position in `buf` that was recognized as part of
 *                              the command string. E.g. if command_tree contains a single
 *                              leaf with name = "mycmd" and user inputted "mycmd 69",
 *                              then lpos will point to the '6' character in buf.
 *
 * @return      A pointer to the HglCommand in `command_tree` that corresponds to the
 *              user input.
 */
const HglCommand *hgl_cmd_input(const HglCommand *command_tree, const char *prompt,
                                char *buf, size_t buf_size, size_t *lpos);

/**
 * Pretty-prints the command tree `command_tree`. `indent` is the initial indentation
 * used when printing the tree.
 */
void hgl_cmd_tree_print(const HglCommand *command_tree, int indent);

/**
 * Returns a pointer to the HglCommand at the specified path in the tree.
 *
 * Example usage: hgl_cmd_tree_at(&cmd_tree, "open", "door");
 */
HglCommand *hgl_cmd_tree_at(HglCommand *command_tree, ...);

/**
 * Returns true if `cmd` is not NULL, is of kind HGL_CMD_LEAF
 * and the private_data member is not NULL.
 */
bool hgl_cmd_has_private_data(const HglCommand *cmd);

/**
 * Returns true if `cmd` is not NULL, is of kind HGL_CMD_NODE
 * and the sub_tree member is not NULL.
 */
bool hgl_cmd_has_sub_tree(const HglCommand *cmd);

#endif /* HGL_CMD_H */

#ifdef HGL_CMD_IMPLEMENTATION

/*============= subset of dynamic array implementation from hgl_da.h ============*/
#include <assert.h>
#if !defined(HGL_ALLOC) && \
    !defined(HGL_REALLOC) && \
    !defined(HGL_FREE)
#include <stdlib.h>
#define HGL_ALLOC malloc
#define HGL_REALLOC realloc
#define HGL_FREE free
#endif

#define HglDynamicArray_(T) \
    struct {               \
        T *arr;            \
        size_t length;     \
        size_t capacity;   \
    }

#ifndef HGL_DA_INITIAL_CAPACITY_
#define HGL_DA_INITIAL_CAPACITY_ 64
#endif

#ifndef HGL_DA_GROWTH_RATE_
#define HGL_DA_GROWTH_RATE_ 1.5
#endif

#define hgl_da_push_(da, item)                                                       \
    do {                                                                             \
        if ((da)->arr == NULL) {                                                     \
            (da)->length = 0;                                                        \
            (da)->capacity = HGL_DA_INITIAL_CAPACITY_;                               \
            (da)->arr = HGL_ALLOC((da)->capacity * sizeof(*(da)->arr));              \
        }                                                                            \
        if ((da)->capacity < ((da)->length + 1)) {                                   \
            (da)->capacity *= HGL_DA_GROWTH_RATE_;                                   \
            (da)->arr = HGL_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr)); \
        }                                                                            \
        assert(((da)->arr != NULL) && "[hgl] Error: (re)alloc failed");              \
        (da)->arr[(da)->length++] = (item);                                          \
    } while (0)

#define hgl_da_pop_(da) ((da)->arr[--(da)->length])

#define hgl_da_at_(da, i) ((da)->arr[((i % (ssize_t)(da)->length) +                  \
                          (da)->length) % (da)->length])

#define hgl_da_free_(da)                                                             \
    do {                                                                             \
        HGL_FREE((da)->arr);                                                         \
    } while (0)


/*===============================================================================*/

#include <stdio.h>
#include <termios.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

const HglCommand *hgl_cmd_input(const HglCommand *command_tree, const char *prompt,
                                char *buf, size_t buf_size, size_t *lpos)
{
    assert(command_tree != NULL);
    assert(prompt != NULL);
    assert(buf != NULL);

    int c = 0;
    size_t buf_idx = 0;
    const HglCommand *current_subtree = command_tree;
    HglDynamicArray_(const HglCommand *) cmd_sequence  = {0};
    HglDynamicArray_(size_t) cmd_indices = {0};
    hgl_da_push_(&cmd_indices, 0);

    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    for (buf_idx = 0; buf_idx < buf_size; buf_idx++) {
        c = buf[buf_idx];
        if (c == '\0') {
            break;
        }

        if (c == ' ') {
            size_t buf_idx = 0;
            HglCommand *cmd = (HglCommand *) &current_subtree[buf_idx];
            size_t word_start = hgl_da_at_(&cmd_indices, -1);
            while (true) {
                cmd = (HglCommand *) &current_subtree[buf_idx++];
                if (cmd->kind == HGL_CMD_NONE) {
                    break;
                }
                if (strlen(cmd->name) != ((buf_idx - 1) - word_start)) {
                    continue;
                }
                if (0 != strncmp(cmd->name, &buf[word_start], (buf_idx - 1) - word_start)) {
                    continue;
                }

                hgl_da_push_(&cmd_sequence, cmd);
                hgl_da_push_(&cmd_indices, (buf_idx - 1));
                current_subtree = (cmd->kind == HGL_CMD_NODE) ? cmd->sub_tree : NULL;
                break;
            }
        }
    }

    while (true) {
        printf("\33[2K\r%s%s", prompt, buf);

        bool double_tab = (c == '\t');
        c = fgetc(stdin);
        double_tab = double_tab && (c == '\t');

        switch (c) {

            case 127: /* DEL */ {
                if (buf_idx > 0) {
                    buf[--buf_idx] = '\0';
                }
                if(hgl_da_at_(&cmd_indices, -1) > buf_idx) {
                    (void) hgl_da_pop_(&cmd_indices);
                    (void) hgl_da_pop_(&cmd_sequence);
                    if (cmd_sequence.length > 0) {
                        current_subtree = hgl_da_at_(&cmd_sequence, -1)->sub_tree;
                    } else {
                        current_subtree = command_tree;
                    }
                }
            } break;

            case '\n': case '\r': {
                /* restore the original terminal attributes */
                tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
                if (lpos != NULL && cmd_indices.length > 0) {
                    *lpos = hgl_da_at_(&cmd_indices, -1);
                }
                const HglCommand *cmd = (cmd_sequence.length > 0) ?
                                        hgl_da_at_(&cmd_sequence, -1) :
                                        NULL;
                hgl_da_free_(&cmd_sequence);
                hgl_da_free_(&cmd_indices);
                printf("\n");
                return cmd;
            } break;

            case '\t': {
                if (current_subtree == NULL) {
                    break;
                }

                /* find matches */
                const HglCommand *match = NULL;
                HglDynamicArray_(size_t) matches = {0};
                size_t i = 0;
                HglCommand cmd = current_subtree[i];
                size_t word_start = hgl_da_at_(&cmd_indices, -1);
                while (cmd.kind != HGL_CMD_NONE) {
                    if (0 == strncmp(cmd.name, &buf[word_start], buf_idx - word_start)) {
                        hgl_da_push_(&matches, i);
                    }
                    cmd = current_subtree[++i];
                }

                /* get single match or print multiple on double tab */
                if (matches.length == 1) {
                    match = &current_subtree[matches.arr[0]];
                } else if ((matches.length > 1) && double_tab) {
                    printf("\n");
                    int cols = 5;
                    int rows = matches.length / cols;
                    int rem  = matches.length % cols;
                    for (int i = 0; i < rows; i++) {
                        for (int j = 0; j < cols; j++) {
                            printf("  %-5s", current_subtree[matches.arr[cols*i + j]].name);
                        }
                        printf("\n");
                    }
                    if (rem != 0) {
                        for (int i = 0; i < rem; i++) {
                            printf("  %-5s ", current_subtree[matches.arr[cols*rows + i]].name);
                        }
                        printf("\n");
                    }
                }

                hgl_da_free_(&matches);

                if (match != NULL) {
                    size_t len = strlen(match->name);
                    assert((word_start + len) < buf_size);
                    memcpy(&buf[word_start], match->name, len);
                    buf_idx = word_start + len;
                    hgl_da_push_(&cmd_sequence, match);
                    buf[buf_idx++] = ' ';
                    hgl_da_push_(&cmd_indices, buf_idx);
                    current_subtree = (match->kind == HGL_CMD_NODE) ? match->sub_tree : NULL;
                }
            } break;

            default: {
                /* discard non-alphanumeric */
                if (c < 0x20 || c > 0x7F) {
                    break;
                }

                if (buf_idx < buf_size) {
                    buf[buf_idx++] = c;
                }

                /* find matches */
                if (current_subtree == NULL) {
                    break;
                }
                size_t i = 0;
                HglCommand *cmd = (HglCommand *) &current_subtree[i];
                size_t word_start = hgl_da_at_(&cmd_indices, -1);
                while (true) {
                    cmd = (HglCommand *) &current_subtree[i++];
                    if (cmd->kind == HGL_CMD_NONE) {
                        break;
                    }
                    if (strlen(cmd->name) != (buf_idx - word_start)) {
                        continue;
                    }
                    if (0 != strncmp(cmd->name, &buf[word_start], buf_idx - word_start)) {
                        continue;
                    }

                    hgl_da_push_(&cmd_sequence, cmd);
                    buf[buf_idx++] = ' ';
                    hgl_da_push_(&cmd_indices, buf_idx);
                    current_subtree = (cmd->kind == HGL_CMD_NODE) ? cmd->sub_tree : NULL;
                    break;
                }
            } break;
        }
    }

}

HglCommand *hgl_cmd_tree_at(HglCommand *command_tree, ...)
{
    HglCommand *cmd = NULL;
    va_list path;
    va_start(path, command_tree);
    while (true)
    {
        const char *arg = va_arg(path, const char *);
        if ((arg == NULL) || (strlen(arg) == 0)) {
            break;
        }

        fflush(stdout);
        size_t i = 0;
        while(true) {
            cmd = &command_tree[i++];
            if (cmd->kind == HGL_CMD_NONE) {
                return NULL;
            }
            if ((cmd->kind == HGL_CMD_NODE) && (0 == strcmp(arg, cmd->name))) {
                command_tree = (HglCommand *) cmd->sub_tree;
                break;
            }
            if ((cmd->kind == HGL_CMD_LEAF) && (0 == strcmp(arg, cmd->name))) {
                return cmd;
            }
        }
    }
    va_end(path);

    return cmd;
}

void hgl_cmd_tree_print(const HglCommand *command_tree, int indent)
{
    int i = 0;
    HglCommand cmd = command_tree[i++];
    while (cmd.kind != HGL_CMD_NONE) {
        printf("%*s%*s %s\n", indent, "", -24 + indent, cmd.name, cmd.desc);
        if (cmd.kind == HGL_CMD_NODE) {
            hgl_cmd_tree_print(cmd.sub_tree, indent + 4);
        }
        cmd = command_tree[i++];
    }
}

bool hgl_cmd_has_private_data(const HglCommand *cmd)
{
    return (cmd != NULL) &&
           (cmd->kind == HGL_CMD_LEAF) &&
           (cmd->private_data != NULL);
}

bool hgl_cmd_has_sub_tree(const HglCommand *cmd)
{
    return (cmd != NULL) &&
           (cmd->kind == HGL_CMD_NODE) &&
           (cmd->sub_tree != NULL);
}

#endif /* HGL_CMD_IMPLEMENTATION */

