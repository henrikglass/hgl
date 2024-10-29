
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
 * Note: The name of a command must not contain any whitespace. This can be verified
 * for the entire tree by calling `hgl_cmd_tree_verify`.
 *
 * You may provide a custom allocator hgl_cmd.h by redefining the following:
 *
 *     HGL_CMD_ALLOC
 *     HGL_CMD_REALLOC
 *     HGL_CMD_FREE
 *
 * User input example:
 *
 *     #define HGL_CMD_BUFFER_SIZE 64 // optional
 *     #define HGL_CMD_IMPLEMENTATION
 *     #include "hgl_cmd.h"
 *
 *     const HglCommand *cmd = hgl_cmd_input(cmd_tree, ">>> ", NULL);
 *     if (cmd == hgl_cmd_tree_at(cmd_tree, "say", "hello")) {
 *         printf("Hello World!\n");
 *     } else if (cmd == hgl_cmd_tree_at(cmd_tree, "say", "goodbye")) {
 *         printf("Goodbye folks!\n");
 *     }
 *
 * The caller or `hgl_cmd_input` may parse additional data in the supplied input
 * buffer (`buf` in this case) by providing a pointer to `char *` as the last
 * argument (args). Upon returning, `hgl_cmd_input` will set args to point to
 * the first byte inside `buf` which is not part of the string corresponding
 * with a certain path in the command tree (`hgl_cmd_input` returns a pointer to
 * this command).
 *
 * See the test/ directory for a more complete example.
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

#ifndef HGL_CMD_BUFFER_SIZE
#define HGL_CMD_BUFFER_SIZE 256
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
 * @param[out]  args            A pointer to the first byte of the input string that was
 *                              not recognized as part of a command. E.g. if command_tree
 *                              contains a single leaf with name = "mycmd" and user
 *                              inputted "mycmd 69", then args will point to the '6'
 *                              character.
 *
 * @return      A pointer to the HglCommand in `command_tree` that corresponds to the
 *              user input.
 */
const HglCommand *hgl_cmd_input(const HglCommand *command_tree,
                                const char *prompt,
                                const char **args);

/**
 * Clears (and frees) history.
 */
void hgl_cmd_clear_history();

/**
 * Verifies that the tree is valid (For now, that names follow the no whitespace rule).
 * Passing an invalid tree will terminate the program with a descriptive message.
 */
void hgl_cmd_tree_verify(const HglCommand *command_tree);

/**
 * Pretty-prints the command tree `command_tree`. `indent` is the initial indentation
 * used when printing the tree.
 */
void hgl_cmd_tree_print(const HglCommand *command_tree, int indent, int desc_margin);

/**
 * Returns a pointer to the HglCommand at the specified path in the tree, or NULL, if
 * no such command exists. The path is supplied as a variadic arguments list.
 *
 * Example usage: hgl_cmd_tree_at(&cmd_tree, "open", "door");
 */
#define hgl_cmd_tree_at(command_tree, ...) (hgl_cmd_tree_at_((command_tree), __VA_ARGS__, NULL))
HglCommand *hgl_cmd_tree_at_(const HglCommand *command_tree, ...);

/**
 * Returns a pointer to the HglCommand at the specified path in the tree, or NULL, if
 * no such command exists. The path is supplied as an array of C-strings `path` with
 * `len` elements. `*end_index` is set to the index of the first element of `path` after the
 * prefix that was matched against the `command_tree` structure. If `*end_idx` == `len`,
 * then no suffix exists. This might be useful if the path contains a suffix that you want to
 * parse separately, e.g. if the command takes a list of arguments.
 *
 * Example usage: hgl_cmd_tree_at(&cmd_tree, &argv[1], argc - 1, &end_idx);
 */
HglCommand *hgl_cmd_tree_at_argv(const HglCommand *command_tree,
                                 const char *path[],
                                 size_t len,
                                 size_t *end_idx);

/**
 * Returns a pointer to the HglCommand at the specified path in the tree, or NULL, if
 * no such command exists. The path is supplied as a null-terminated string `path`.
 * `*end` is set to point at the first character of `path` after the prefix that was
 * matched against the `command_tree` structure. This might be useful if the path
 * contains a suffix that you want to parse separately, e.g. if the command takes a
 * list of arguments.
 *
 * Example usage: hgl_cmd_tree_at(&cmd_tree, "open door", &end);
 */
HglCommand *hgl_cmd_tree_at_cstr(const HglCommand *command_tree, const char *path, const char **end);

/**
 * Returns a pointer to the child of `parent` with the name `child_name`, or NULL
 * if no such child exists.
 */
HglCommand *hgl_cmd_tree_get_child(const HglCommand *parent, const char *child_name);

/**
 * Returns true if `command_tree` is an ancestor of `cmd`. Essentially performs DFS
 * startning at `command_tree`.
 */
bool hgl_cmd_is_descendant(const HglCommand *command_tree, const HglCommand *cmd);

#endif /* HGL_CMD_H */

#ifdef HGL_CMD_IMPLEMENTATION

/*============= subset of dynamic array implementation from hgl_da.h ============*/
#include <assert.h>
#if !defined(HGL_CMD_ALLOC) && \
    !defined(HGL_CMD_REALLOC) && \
    !defined(HGL_CMD_FREE)
#include <stdlib.h>
#define HGL_CMD_ALLOC malloc
#define HGL_CMD_REALLOC realloc
#define HGL_CMD_FREE free
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

#define hgl_da_push_(da, item)                                                           \
    do {                                                                                 \
        if ((da)->arr == NULL) {                                                         \
            (da)->length = 0;                                                            \
            (da)->capacity = HGL_DA_INITIAL_CAPACITY_;                                   \
            (da)->arr = HGL_CMD_ALLOC((da)->capacity * sizeof(*(da)->arr));              \
        }                                                                                \
        if ((da)->capacity < ((da)->length + 1)) {                                       \
            (da)->capacity *= HGL_DA_GROWTH_RATE_;                                       \
            (da)->arr = HGL_CMD_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr)); \
        }                                                                                \
        assert(((da)->arr != NULL) && "[hgl] Error: (re)alloc failed");                  \
        (da)->arr[(da)->length++] = (item);                                              \
    } while (0)

#define hgl_da_pop_(da) ((da)->arr[--(da)->length])

#define hgl_da_at_(da, i) ((da)->arr[((i % (ssize_t)(da)->length) +                      \
                          (da)->length) % (da)->length])

#define hgl_da_free_(da)                                                                 \
    do {                                                                                 \
        HGL_CMD_FREE((da)->arr);                                                         \
    } while (0)


/*===============================================================================*/

#include <stdio.h>
#include <termios.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

static HglDynamicArray_(char *) hgl_cmd_history_ = {0};
char hgl_temp_buf_[HGL_CMD_BUFFER_SIZE];

const HglCommand *hgl_cmd_input(const HglCommand *command_tree,
                                const char *prompt,
                                const char **args)
{
    /* allocate new buffer for this command*/
    char *buf = HGL_CMD_ALLOC(HGL_CMD_BUFFER_SIZE);
    memset(buf, 0, HGL_CMD_BUFFER_SIZE);
    size_t history_idx = hgl_cmd_history_.length;

    assert(buf != NULL);
    assert(command_tree != NULL);
    assert(prompt != NULL);

    int c = 0;
    size_t buf_idx = 0;
    const HglCommand *current_subtree = command_tree;
    HglDynamicArray_(const HglCommand *) cmd_sequence  = {0};
    HglDynamicArray_(size_t) cmd_indices = {0};
    hgl_da_push_(&cmd_indices, 0);

    /* set the terminal to raw mode */
    struct termios orig_term_attr;
    struct termios new_term_attr;
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    bool running = true;

    while (running) {
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
            } continue; /* skip match finding step */

            case '\33': /* arrow keys */ {

                char c1 = fgetc(stdin);
                char c2 = fgetc(stdin);

                if (hgl_cmd_history_.length == 0) {
                    break;
                }

                if (history_idx == hgl_cmd_history_.length) {
                    memcpy(hgl_temp_buf_, buf, HGL_CMD_BUFFER_SIZE);
                }

                if (c1 == '[' && c2 == 'A') {
                    history_idx = (history_idx > 0) ? history_idx - 1 : 0;
                } else if (c1 == '[' && c2 == 'B') {
                    history_idx = (history_idx < hgl_cmd_history_.length) ?
                                  history_idx + 1 :
                                  hgl_cmd_history_.length;
                } else {
                    break;
                }

                if (history_idx == hgl_cmd_history_.length) {
                    memcpy(buf, hgl_temp_buf_, HGL_CMD_BUFFER_SIZE);
                } else {
                    memcpy(buf, hgl_cmd_history_.arr[history_idx], HGL_CMD_BUFFER_SIZE);
                }

                /* re-parse buffer */
                current_subtree = command_tree;
                cmd_sequence.length = 0;
                cmd_indices.length = 0;
                hgl_da_push_(&cmd_indices, 0);

                size_t word_start = 0;
                for (buf_idx = 0; buf_idx < HGL_CMD_BUFFER_SIZE; buf_idx++) {
                    char c = buf[buf_idx];
                    if (c == '\0') {
                        break;
                    }

                    if (c == ' ' && current_subtree != NULL) {
                        size_t i = 0;
                        while (true) {
                            const HglCommand *cmd = (HglCommand *) &current_subtree[i++];
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
                            current_subtree = (cmd->kind == HGL_CMD_NODE) ? cmd->sub_tree : NULL;
                            while(buf[buf_idx] == ' ') {
                                buf_idx++;
                            }
                            word_start = buf_idx;
                            hgl_da_push_(&cmd_indices, word_start);
                            buf_idx--;
                            break;
                        }
                    }
                }
            } break;

            case '\n': case '\r': {
                running = false;
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

                if (matches.length == 1) {
                    /* autocomplete match */
                    match = &current_subtree[matches.arr[0]];
                } else if ((matches.length > 1) && !double_tab) {
                    /* autocomplete largest common prefix in matches */
                    size_t lcp = 1000; // something big
                    for (size_t i = 0; i < matches.length - 1; i++) {
                        size_t pair_lcp = 0;
                        for (size_t j = 0; ; j++) {
                            if (current_subtree[matches.arr[i]].name[j] == '\0') {
                                break;
                            }

                            if (current_subtree[matches.arr[i + 1]].name[j] == '\0') {
                                break;
                            }

                            if (current_subtree[matches.arr[i]].name[j] !=
                                current_subtree[matches.arr[i + 1]].name[j]) {
                                break;
                            }

                            pair_lcp++;
                        }
                        lcp = (pair_lcp < lcp) ? pair_lcp : lcp;
                    }
                    memcpy(&buf[word_start], current_subtree[matches.arr[0]].name, lcp);
                    buf_idx = word_start + lcp;
                } else if ((matches.length > 1) && double_tab) {

                    /* find length of longest matching string */
                    int longest_match_len = 0;
                    for (size_t i = 0; i < matches.length; i++) {
                        const char *name = current_subtree[matches.arr[i]].name;
                        int length = (int) strlen(name);
                        longest_match_len = (length > longest_match_len) ? length : longest_match_len;
                    }

                    /* display matching cmds */
                    printf("\n");
                    int cols = 80 / longest_match_len;
                    int rows = matches.length / cols;
                    int rem  = matches.length % cols;
                    for (int i = 0; i < rows; i++) {
                        for (int j = 0; j < cols; j++) {
                            printf("  %*s", -longest_match_len,
                                   current_subtree[matches.arr[cols*i + j]].name);
                        }
                        printf("\n");
                    }
                    if (rem != 0) {
                        for (int i = 0; i < rem; i++) {
                            printf("  %*s", -longest_match_len,
                                   current_subtree[matches.arr[cols*rows + i]].name);
                        }
                        printf("\n");
                    }
                }

                hgl_da_free_(&matches);

                if (match != NULL) {
                    size_t len = strlen(match->name);
                    assert((word_start + len) < HGL_CMD_BUFFER_SIZE);
                    memcpy(&buf[word_start], match->name, len);
                    buf_idx = word_start + len;
                    hgl_da_push_(&cmd_sequence, match);
                    buf[buf_idx++] = ' ';
                    hgl_da_push_(&cmd_indices, buf_idx);
                    current_subtree = (match->kind == HGL_CMD_NODE) ? match->sub_tree : NULL;
                }
            } continue;

            default: {
                /* discard non-alphanumeric */
                if (c < 0x20 || c > 0x7F) {
                    break;
                }

                if (buf_idx < HGL_CMD_BUFFER_SIZE) {
                    // Temporary fix for pasting
                    if ((buf_idx > 0) && (buf[buf_idx - 1] == ' ') && (c == ' ')) {
                        break;
                    }
                    buf[buf_idx++] = c;
                }
            } break;
        }

        /* find matches */
        if (current_subtree == NULL) {
            continue;
        }
        size_t i = 0;
        size_t word_start = hgl_da_at_(&cmd_indices, -1);
        while (true) {
            HglCommand *cmd = (HglCommand *) &current_subtree[i++];
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
    }

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
    printf("\n");

    /* Return the command corresponding to the command sequence */
    const HglCommand *cmd = (cmd_sequence.length > 0) ?
                             hgl_da_at_(&cmd_sequence, -1) :
                             NULL;

    if (cmd == NULL) {
        hgl_da_free_(&cmd_sequence);
        hgl_da_free_(&cmd_indices);
        HGL_CMD_FREE(buf);
        return NULL;
    }

    hgl_da_push_(&hgl_cmd_history_, buf);
    if (args != NULL && cmd_indices.length > 0) {
        *args = buf + hgl_da_at_(&cmd_indices, -1);
    }
    hgl_da_free_(&cmd_sequence);
    hgl_da_free_(&cmd_indices);

    return cmd;
}

void hgl_cmd_clear_history()
{
    for (size_t i = 0; i < hgl_cmd_history_.length; i++) {
        HGL_CMD_FREE(hgl_cmd_history_.arr[i]);
    }
    hgl_cmd_history_.length = 0;
}

HglCommand *hgl_cmd_tree_at_(const HglCommand *command_tree, ...)
{
    HglCommand *cmd = NULL;
    va_list path;
    va_start(path, command_tree);
    while (true) {
        const char *arg = va_arg(path, const char *);
        if ((arg == NULL) || (strlen(arg) == 0)) {
            break;
        }

        cmd = hgl_cmd_tree_get_child(command_tree, arg);

        if (cmd == NULL) {
            return NULL;
        }

        if (cmd->kind == HGL_CMD_NODE) {
            command_tree = cmd->sub_tree;
        } else {
            break;
        }
    }
    va_end(path);

    return cmd;
}

HglCommand *hgl_cmd_tree_at_argv(const HglCommand *command_tree,
                                 const char *path[],
                                 size_t len,
                                 size_t *end_idx)
{
    HglCommand *cmd = NULL;
    const char *arg;

    for (size_t i = 0; i < len; i++) {
        arg = path[i];

        cmd = hgl_cmd_tree_get_child(command_tree, arg);

        if (cmd == NULL) {
            *end_idx = 0;
            return NULL;
        }

        if (cmd->kind == HGL_CMD_NODE) {
            command_tree = cmd->sub_tree;
        } else {
            *end_idx = i + 1;
            break;
        }
    }

    return (HglCommand *) cmd;
}

HglCommand *hgl_cmd_tree_at_cstr(const HglCommand *command_tree, const char *path, const char **end)
{
    HglCommand *cmd = NULL;

    const char *arg = path;
    size_t arglen = 0;

    while (true) {
        /* extract next argument */
        while (true) {
            if (arg[arglen] == ' ' || arg[arglen] == '\0') {
                break;
            }
            arglen++;
        }

        *end = arg + arglen;

        if (arglen == 0) {
            break;
        }

        /* get child */
        size_t i = 0;
        while(true) {
            cmd = (HglCommand *) &command_tree[i++];

            if (cmd->kind == HGL_CMD_NONE) {
                return NULL;
            }

            if ((cmd->kind == HGL_CMD_NODE) && (0 == strncmp(arg, cmd->name, arglen))) {
                command_tree = (HglCommand *) cmd->sub_tree;
                break;
            }

            if (0 == strncmp(arg, cmd->name, arglen)) {
                return cmd;
            }
        }

        /* step to next argument */
        arg += arglen + 1;
        arglen = 0;

    }

    return cmd;

}

HglCommand *hgl_cmd_tree_get_child(const HglCommand *command_tree, const char *child_name)
{
    size_t i = 0;
    while(true) {
        HglCommand *child = (HglCommand *) &command_tree[i++];

        if (child->kind == HGL_CMD_NONE) {
            return NULL;
        }

        if (0 == strcmp(child_name, child->name)) {
            return child;
        }
    }
}

bool hgl_cmd_is_descendant(const HglCommand *command_tree, const HglCommand *cmd)
{

    /* None-kind nodes and leaves have no descendents */
    if (command_tree->kind != HGL_CMD_NODE) {
        return false;
    }

    /* Say `cmd` is an ancestor of itself. Why not. */
    if (cmd == command_tree) {
        return true;
    }

    size_t i = 0;
    while (true) {
        const HglCommand *next_ancestor = &command_tree->sub_tree[i++];

        /* If we reach the end of the subtree, no match has been found */
        if (next_ancestor->kind == HGL_CMD_NONE) {
            return false;
        }

        /* found descendent */
        if (cmd == next_ancestor) {
            return true;
        }

        /* Check subtree */
        if ((next_ancestor->kind == HGL_CMD_NODE) &&
            (hgl_cmd_is_descendant(next_ancestor, cmd))) {
            return true;
        }
    }
}

void hgl_cmd_tree_verify(const HglCommand *command_tree)
{
    bool tree_invalid = false;
    int i = 0;
    while (true) {
        HglCommand *cmd = (HglCommand *) &command_tree[i++];

        if (cmd->kind == HGL_CMD_NONE) {
            break;
        }

        int j = 0;
        while (true) {

            if (cmd->name[j] == '\0') {
                break;
            }

            if (cmd->name[j] == ' ') {
                tree_invalid = true;
                printf("[hgl_cmd_tree_verify] Error: Command names must not "
                       "contain spaces \"%s\"\n", cmd->name);
            }

            j++;
        }

        if (cmd->kind == HGL_CMD_NODE) {
            hgl_cmd_tree_verify(cmd->sub_tree);
        }
    }

    if (tree_invalid) {
        printf("Aborting...\n");
        abort();
    }
}

void hgl_cmd_tree_print(const HglCommand *command_tree, int indent, int desc_margin)
{
    if (desc_margin == 0) {
        desc_margin = 48;
    }
    int i = 0;
    HglCommand cmd = command_tree[i++];
    while (cmd.kind != HGL_CMD_NONE) {
        printf("%*s%*s %s\n", indent, "", -desc_margin + indent, cmd.name, cmd.desc);
        if (cmd.kind == HGL_CMD_NODE) {
            hgl_cmd_tree_print(cmd.sub_tree, indent + 4, desc_margin);
        }
        cmd = command_tree[i++];
    }
}

#endif /* HGL_CMD_IMPLEMENTATION */

