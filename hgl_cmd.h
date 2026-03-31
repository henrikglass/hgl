
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2026 Henrik A. Glass
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
 *     #define HGL_CMD_GAP_BUFFER_SIZE 512 // optional
 *     #define HGL_CMD_HISTORY_BUFFER_SIZE 1024*1024 // optional
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
 * See the examples/ directory for a more complete example.
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

#ifndef HGL_CMD_GAP_BUFFER_SIZE
#define HGL_CMD_GAP_BUFFER_SIZE 4096
#endif

#ifndef HGL_CMD_HISTORY_BUFFER_SIZE
#define HGL_CMD_HISTORY_BUFFER_SIZE 64*1024
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
 * @param       cmd_tree_root   A command tree.
 * @param       prompt          An optional prompt prefix, e.g. ">>> " or "$ ".
 * @param[out]  args            A pointer to the first byte of the input string that was
 *                              not recognized as part of a command. E.g. if cmd_tree_root
 *                              contains a single leaf with name = "mycmd" and user
 *                              inputted "mycmd 69", then args will point to the '6'
 *                              character.
 *
 * @return      A pointer to the HglCommand in `cmd_tree_root` that corresponds to the
 *              user input.
 */
const HglCommand *hgl_cmd_input(const HglCommand *cmd_tree_root,
                                const char *prompt,
                                const char **args);

/**
 * Verifies that the tree is valid (For now, that names follow the no whitespace rule).
 * Passing an invalid tree will terminate the program with a descriptive message.
 */
void hgl_cmd_tree_verify(const HglCommand *cmd_tree_root);

/**
 * Pretty-prints the command tree `cmd_tree_root`. `indent` is the initial indentation
 * used when printing the tree.
 */
void hgl_cmd_tree_print(const HglCommand *cmd_tree_root, int indent, int desc_margin);

/**
 * Returns a pointer to the HglCommand at the specified path in the tree, or NULL, if
 * no such command exists. The path is supplied as a variadic arguments list.
 *
 * Example usage: hgl_cmd_tree_at(&cmd_tree, "open", "door");
 */
#define hgl_cmd_tree_at(cmd_tree_root, ...) (hgl_cmd_tree_at_((cmd_tree_root), __VA_ARGS__, NULL))
HglCommand *hgl_cmd_tree_at_(const HglCommand *cmd_tree_root, ...);

/**
 * Returns a pointer to the HglCommand at the specified path in the tree, or NULL, if
 * no such command exists. The path is supplied as an array of C-strings `path` with
 * `len` elements. `*end_index` is set to the index of the first element of `path` after the
 * prefix that was matched against the `cmd_tree_root` structure. If `*end_idx` == `len`,
 * then no suffix exists. This might be useful if the path contains a suffix that you want to
 * parse separately, e.g. if the command takes a list of arguments.
 *
 * Example usage: hgl_cmd_tree_at(&cmd_tree, &argv[1], argc - 1, &end_idx);
 */
HglCommand *hgl_cmd_tree_at_argv(const HglCommand *cmd_tree_root,
                                 const char *path[],
                                 size_t len,
                                 size_t *end_idx);

/**
 * Returns a pointer to the HglCommand at the specified path in the tree, or NULL, if
 * no such command exists. The path is supplied as a null-terminated string `path`.
 * `*end` is set to point at the first character of `path` after the prefix that was
 * matched against the `cmd_tree_root` structure. This might be useful if the path
 * contains a suffix that you want to parse separately, e.g. if the command takes a
 * list of arguments.
 *
 * Example usage: hgl_cmd_tree_at_cstr(&cmd_tree, "open door", &end);
 */
HglCommand *hgl_cmd_tree_at_cstr(const HglCommand *cmd_tree_root, const char *path, const char **end);

/**
 * Similar to hgl_cmd_tree_at_cstr, except it will only return a match for a unique 
 * matching prefix. I.e. if `cmd_tree` has two paths: "open door" and "open door2",
 * then the below example will return a pointer to "open" instead of "open door".
 *
 * Example usage: hgl_cmd_tree_at_cstr_unique(&cmd_tree, "open door", &end);
 */
HglCommand *hgl_cmd_tree_at_cstr_unique(const HglCommand *cmd_tree_root, const char *path, const char **end);

/**
 * Returns a pointer to the child of `parent` with the name `child_name`, or NULL
 * if no such child exists.
 */
HglCommand *hgl_cmd_tree_get_child(const HglCommand *parent, const char *child_name);

/**
 * Returns true if `cmd_tree_root` is an ancestor of `cmd`. Essentially performs DFS
 * startning at `cmd_tree_root`.
 */
bool hgl_cmd_is_descendant(const HglCommand *cmd_tree_root, const HglCommand *cmd);

#endif /* HGL_CMD_H */

#ifdef HGL_CMD_IMPLEMENTATION

/*--- Include files ---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <assert.h>
#include <sys/types.h>
#include <ctype.h>

/*--- Private macros --------------------------------------------------------------------*/

#if !defined(HGL_CMD_ALLOC) && \
    !defined(HGL_CMD_REALLOC) && \
    !defined(HGL_CMD_FREE)
#include <stdlib.h>
#define HGL_CMD_ALLOC malloc
#define HGL_CMD_REALLOC realloc
#define HGL_CMD_FREE free
#endif

#define HISTORY_PAST_END 0xFFFFFFFFu

/*--- Private type definitions ----------------------------------------------------------*/

/**
 *
 * Cursor index:  0 1 2 3 4           4 5 6 7
 *               [X,X,X,X,_,_,_,_,_,_,Y,Y,Y,Y]
 *                        ^           ^
 *                        |           |
 *                        |           |
 *                        S           E
 *
 * where size == 8, gap_start = 4
 */
typedef struct {
    char * const data; 
    int size; 
    int gap_start;
    int gap_end;
    int cursor;
} GapBuffer;

typedef struct {
    const char **arr;
    size_t count;
    size_t capacity;
    int longest_match;
    int longest_common_prefix;
} MatchBuffer;

typedef struct {
    void *strs; 
    int count;
    int stride;
} ComplBuffer;

typedef struct {
    uint8_t *data; 
    uint32_t size;
    uint32_t first; 
    uint32_t last; 
    uint32_t nav;
    bool is_empty;
} HistBuffer;

typedef struct {
    uint32_t prev; 
    uint32_t next; 
    uint32_t length;
} HistEntryHeader;

/*--- Private function prototypes -------------------------------------------------------*/

static GapBuffer gb_make(char * const buf, size_t size);
static void gb_clear(GapBuffer *gbuf);
static void gb_load_contents(GapBuffer *gbuf, const char *str, size_t length);
static void gb_insert_char(GapBuffer *gbuf, char c);
static char *gb_to_cstr(GapBuffer *gbuf, char *buf);
static int gb_length(GapBuffer *gbuf);
static char gb_char_left_of_cursor(GapBuffer *gbuf);
static char gb_char_at_cursor(GapBuffer *gbuf);
static void gb_position_split_on_cursor(GapBuffer *gbuf);
static void gb_insert_char(GapBuffer *gbuf, char c);
static void gb_insert_many(GapBuffer *gbuf, const char *str, int n);
static int gb_step_cursor(GapBuffer *gbuf, int d);
static void gb_set_cursor(GapBuffer *gbuf, int i);
static void gb_delete_right(GapBuffer *gbuf, int n);
static void gb_delete_left(GapBuffer *gbuf, int n);
static void gb_delete_left_all(GapBuffer *gbuf);
static void gb_delete_right_all(GapBuffer *gbuf);
static int gb_word_under_cursor(GapBuffer *gbuf, char *buf);
static void gb_display(GapBuffer *gbuf, const char *prompt);
#if 0
static void gb_insert_cstr(GapBuffer *gbuf, const char *cstr);
#endif
#if 0
static void gb_debug_print(GapBuffer *gbuf);
#endif

static void mb_push(MatchBuffer *mbuf, const char *cstr);
static void mb_clear(MatchBuffer *mbuf);
static void mb_free(MatchBuffer *mbuf);
static void mb_display(MatchBuffer *mbuf);

static ComplBuffer cb_make(void *strs, int count, int stride);
static ComplBuffer cb_make_from_cmd_tree(const HglCommand *cmd_tree);
static void cb_match_prefix(ComplBuffer *cbuf, const char *prefix, MatchBuffer *matches);
#if 0
static void cb_debug_print(ComplBuffer *cbuf);
#endif

static void hb_append(HistBuffer *hbuf, const char *str, size_t length);
static const char *hb_navigate_prev(HistBuffer *hbuf, uint32_t *length);
static const char *hb_navigate_next(HistBuffer *hbuf, uint32_t *length);
#if 0
static HistBuffer hb_make(void *buffer, size_t size);
#endif
#if 0
static void hb_debug_print(const HistBuffer *hbuf);
#endif

/*--- Private variables -----------------------------------------------------------------*/

// persistent global hbuf
static uint8_t hbuf_data_[HGL_CMD_HISTORY_BUFFER_SIZE];
static HistBuffer hbuf_ = {
    .data     = hbuf_data_,
    .size     = HGL_CMD_HISTORY_BUFFER_SIZE,
    .first    = 0u,
    .last     = 0u,
    .nav      = HISTORY_PAST_END,
    .is_empty = true,
};

/*--- Public functions ------------------------------------------------------------------*/

const HglCommand *hgl_cmd_input(const HglCommand *cmd_tree_root,
                                const char *prompt,
                                const char **args)
{
    static char gbuf_data[HGL_CMD_GAP_BUFFER_SIZE];
    static char scratch[HGL_CMD_GAP_BUFFER_SIZE];
    static char history_scratch[HGL_CMD_GAP_BUFFER_SIZE];
    unsigned char c = '\0';
    const char *end = NULL;
    bool running = true;
    bool double_tab = false;
    const HglCommand *curr_tree = (const HglCommand *)cmd_tree_root;
    HglCommand *curr_cmd = NULL;

    /* misc setup */
    history_scratch[0] = '\0';
    gbuf_data[HGL_CMD_GAP_BUFFER_SIZE - 1] = '\0';

    /* create gbuf + cbuf + mbuf */
    GapBuffer gbuf   = gb_make(gbuf_data, HGL_CMD_GAP_BUFFER_SIZE - 1);
    ComplBuffer cbuf = cb_make_from_cmd_tree(curr_tree);
    MatchBuffer mbuf = {0};
    
    /* set the terminal to raw mode */
    struct termios orig_term_attr;
    struct termios new_term_attr;
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    while (running) {
        gb_display(&gbuf, prompt);

        double_tab = (c == '\t');
        c = fgetc(stdin);
        double_tab = double_tab && (c == '\t');

        switch (c) {
            case 'q': break;

            case 1: /* Ctrl-A, apparently */{
                gb_set_cursor(&gbuf, 0);
            } break;

            case 5: /* Ctrl-E, apparently */{
                gb_set_cursor(&gbuf, gb_length(&gbuf));
            } break;

            case 21: /* Ctrl-U, apparently */{
                gb_delete_left_all(&gbuf);
            } break;

            case 11: /* Ctrl-K, apparently */{
                gb_delete_right_all(&gbuf);
            } break;

            case 126: /* delete */ {
                gb_delete_right(&gbuf, 1);
            } break;

            case 127: /* backspace */{
                gb_delete_left(&gbuf, 1);
            } break;

            case '\33': /* arrow keys */ {
                char c1 = fgetc(stdin);
                char c2 = fgetc(stdin);
                if (c1 == 91 && c2 == 68) { // left
                    gb_step_cursor(&gbuf, -1);
                } else if (c1 == 91 && c2 == 67) { // right
                    gb_step_cursor(&gbuf, 1);
                } else if (c1 == '[' && c2 == 'A') { // up
                    if ((hbuf_.nav == HISTORY_PAST_END) /*&& (gb_length(&gbuf) != 0)*/) {
                        gb_to_cstr(&gbuf, history_scratch);
                    }
                    uint32_t length;
                    const char *str = hb_navigate_prev(&hbuf_, &length);
                    gb_load_contents(&gbuf, str, length);
                } else if (c1 == '[' && c2 == 'B') { // down
                    uint32_t length;
                    const char *str = hb_navigate_next(&hbuf_, &length);
                    if (str == NULL) {
                        gb_load_contents(&gbuf, history_scratch, strlen(history_scratch));
                    } else {
                        gb_load_contents(&gbuf, str, length);
                    }
                }
            } break;

            case 59: /* Ctrl-arrow keys */ {
                int d;
                char x;
                char c1 = fgetc(stdin);
                char c2 = fgetc(stdin);
                if (c1 == '5' && c2 == 'D') { // left
                    /* step left until a non-space character is encountered */
                    x = gb_char_left_of_cursor(&gbuf);
                    while (isspace(x)) {
                        d = gb_step_cursor(&gbuf, -1);
                        x = gb_char_left_of_cursor(&gbuf);
                    }

                    /* continue stepping left until a space character is encountered  */
                    do {
                        d = gb_step_cursor(&gbuf, -1);
                        x = gb_char_left_of_cursor(&gbuf);
                    } while (!isspace(x) && d != 0);
                } else if (c1 == '5' && c2 == 'C') { // right
                    /* step right until a non-space character is encountered */
                    x = gb_char_at_cursor(&gbuf);
                    while (isspace(x)) {
                        d = gb_step_cursor(&gbuf, 1);
                        x = gb_char_at_cursor(&gbuf);
                    }

                    /* continue stepping right until a non-space character is encountered */
                    do {
                        d = gb_step_cursor(&gbuf, 1);
                        x = gb_char_at_cursor(&gbuf);
                    } while(!isspace(x) && d != 0);
                }
            } break;

            case '\t': {
                int n = gb_word_under_cursor(&gbuf, scratch); 
                scratch[n] = '\0';
                //printf("scratch = \"%s\"\n", scratch);
                cb_match_prefix(&cbuf, scratch, &mbuf);
                if ((mbuf.longest_common_prefix > n) || (mbuf.count == 1)) {
                    gb_delete_left(&gbuf, n);
                    gb_insert_many(&gbuf, mbuf.arr[0], mbuf.longest_common_prefix);
                    if (mbuf.count == 1) {
                        gb_insert_char(&gbuf, ' ');
                    }
                } else if ((curr_cmd != NULL) && n == (int)strlen(curr_cmd->name) && 0 == strncmp(scratch, curr_cmd->name, n)) {
                    gb_insert_char(&gbuf, ' ');
                } else if (double_tab) {
                    mb_display(&mbuf);
                }
            } break;

            case '\n': case '\r': {
                running = false;
            } break;

            default: {
                /* insert any ascii printable charactes */
                if (isprint(c)) {
                    gb_insert_char(&gbuf, c); 
                }
            } break;
        }

        /* Update completion buffer */
        char *cmdstr = gb_to_cstr(&gbuf, scratch);
        cmdstr[gbuf.cursor] = '\0';
        //curr_cmd = hgl_cmd_tree_at_cstr(cmd_tree_root, cmdstr, &end);
        curr_cmd = hgl_cmd_tree_at_cstr_unique(cmd_tree_root, cmdstr, &end);
        //if (curr_cmd != NULL && cmdstr[(gbuf.cursor - 1) < 0 ? 0 : (gbuf.cursor - 1)] == ' ') {
        if (curr_cmd != NULL) {
            if (curr_cmd->kind == HGL_CMD_LEAF) {
                curr_tree = NULL;
            } else if (/*(curr_cmd->sub-tree != curr_tree) && */ curr_cmd->kind == HGL_CMD_NODE) {
                curr_tree = curr_cmd->sub_tree;
            }
        } else if (end != &cmdstr[gbuf.cursor] && end != cmdstr) {
            curr_tree = NULL;
        } else {
            curr_tree = cmd_tree_root;
        }
        cbuf = cb_make_from_cmd_tree(curr_tree);
    }

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
    printf("\n");

    mb_free(&mbuf);

    /* get command string */
    char *cmdstr = gb_to_cstr(&gbuf, scratch);
    size_t cmdstr_length = strlen(cmdstr);

    /* append to history */
    if (cmdstr_length > 0) {
        hb_append(&hbuf_, cmdstr, cmdstr_length);
    }

    /* parse command str and return */
    curr_cmd = hgl_cmd_tree_at_cstr(cmd_tree_root, cmdstr, &end);
    if (args != NULL) {
        *args = end;
    }
    return curr_cmd;
}

HglCommand *hgl_cmd_tree_at_(const HglCommand *cmd_tree_root, ...)
{
    HglCommand *cmd = NULL;
    va_list path;
    va_start(path, cmd_tree_root);
    while (true) {
        const char *arg = va_arg(path, const char *);
        if ((arg == NULL) || (strlen(arg) == 0)) {
            break;
        }

        cmd = hgl_cmd_tree_get_child(cmd_tree_root, arg);

        if (cmd == NULL) {
            return NULL;
        }

        if (cmd->kind == HGL_CMD_NODE) {
            cmd_tree_root = cmd->sub_tree;
        } else {
            break;
        }
    }
    va_end(path);

    return cmd;
}

HglCommand *hgl_cmd_tree_at_argv(const HglCommand *cmd_tree_root,
                                 const char *path[],
                                 size_t len,
                                 size_t *end_idx)
{
    HglCommand *cmd = NULL;
    const char *arg;

    for (size_t i = 0; i < len; i++) {
        arg = path[i];

        cmd = hgl_cmd_tree_get_child(cmd_tree_root, arg);

        if (cmd == NULL) {
            *end_idx = 0;
            return NULL;
        }

        if (cmd->kind == HGL_CMD_NODE) {
            cmd_tree_root = cmd->sub_tree;
        } else {
            *end_idx = i + 1;
            break;
        }
    }

    return (HglCommand *) cmd;
}

HglCommand *hgl_cmd_tree_at_cstr(const HglCommand *cmd_tree_root, const char *path, const char **end)
{
    HglCommand *curr_cmd = NULL;
    const HglCommand *curr_cmd_subtree = cmd_tree_root;
    bool found_leaf = false;
    const char *p = path;
    size_t l = 0;

    while (true) {
        /* get next word */
        while (isspace(*p)) p++;
        while (!isspace(p[l]) && p[l] != '\0') l++;
        *end = p;

        if (found_leaf) {
            return curr_cmd;
        }

        /* get child if possible */
        size_t i = 0;
        while(true) {
            HglCommand *c = (HglCommand *) &curr_cmd_subtree[i++];

            if (c->kind == HGL_CMD_NONE) {
                return curr_cmd;
            }

            if ((c->kind == HGL_CMD_NODE) && l == strlen(c->name) && (0 == strncmp(c->name, p, l))) {
                curr_cmd = c;
                curr_cmd_subtree = c->sub_tree;
                break;
            }

            if ((c->kind == HGL_CMD_LEAF) && l == strlen(c->name) && (0 == strncmp(c->name, p, l))) {
                curr_cmd = c;
                found_leaf = true;
                break;
            }
        }

        p += l;
        l = 0;
    }

    assert(0 && "unreachable");
    return curr_cmd; // Unreachable
}

HglCommand *hgl_cmd_tree_at_cstr_unique(const HglCommand *cmd_tree_root, const char *path, const char **end)
{
    HglCommand *curr_cmd = NULL;
    HglCommand *next_cmd = NULL;
    const HglCommand *curr_cmd_subtree = cmd_tree_root;
    bool found_leaf = false;
    bool space_at_end = false;
    int n_matches = 0;
    const char *p = path;
    size_t l = 0;

    while (true) {
        /* get next word */
        while (isspace(*p)) p++;
        while (!isspace(p[l]) && p[l] != '\0') l++;
        space_at_end = isspace(p[l]);
        *end = p;

        if (found_leaf) {
            return curr_cmd;
        }

        /* count matching prefixes */
        size_t i = 0;
        n_matches = 0;
        next_cmd = NULL;
        while(true) {
            HglCommand *c = (HglCommand *) &curr_cmd_subtree[i++];

            if (c->kind == HGL_CMD_NONE) {
                break;
            }

            size_t cmdlen = strlen(c->name);
            if ((l == cmdlen) && (0 == strncmp(c->name, p, l))) {
                next_cmd = c;
            }
            if (l <= cmdlen && (0 == strncmp(c->name, p, l))) {
                n_matches++;
            }
        }

        /* unique match? continue down the tree */
        if (next_cmd != NULL && (n_matches == 1 || space_at_end)) {
            if (next_cmd->kind == HGL_CMD_NODE) {
                curr_cmd = next_cmd;
                curr_cmd_subtree = next_cmd->sub_tree;
            } else if (next_cmd->kind == HGL_CMD_LEAF) {
                curr_cmd = next_cmd;
                found_leaf = true;
                n_matches++;
            } else {
                return curr_cmd;
            }
        } else if (space_at_end) {
            return NULL;
        } else {
            return curr_cmd;
        }

        p += l;
        l = 0;
    }

    assert(0 && "unreachable");
    return curr_cmd; // Unreachable
}


HglCommand *hgl_cmd_tree_get_child(const HglCommand *cmd_tree_root, const char *child_name)
{
    size_t i = 0;
    while(true) {
        HglCommand *child = (HglCommand *) &cmd_tree_root[i++];

        if (child->kind == HGL_CMD_NONE) {
            return NULL;
        }

        if (0 == strcmp(child_name, child->name)) {
            return child;
        }
    }
}

bool hgl_cmd_is_descendant(const HglCommand *cmd_tree_root, const HglCommand *cmd)
{

    /* None-kind nodes and leaves have no descendents */
    if (cmd_tree_root->kind != HGL_CMD_NODE) {
        return false;
    }

    /* Say `cmd` is an ancestor of itself. Why not. */
    if (cmd == cmd_tree_root) {
        return true;
    }

    size_t i = 0;
    while (true) {
        const HglCommand *next_ancestor = &cmd_tree_root->sub_tree[i++];

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

void hgl_cmd_tree_verify(const HglCommand *cmd_tree_root)
{
    bool tree_invalid = false;
    int i = 0;
    while (true) {
        HglCommand *cmd = (HglCommand *) &cmd_tree_root[i++];

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

void hgl_cmd_tree_print(const HglCommand *cmd_tree_root, int indent, int desc_margin)
{
    if (desc_margin == 0) {
        desc_margin = 48;
    }
    int i = 0;
    HglCommand cmd = cmd_tree_root[i++];
    while (cmd.kind != HGL_CMD_NONE) {
        printf("%*s%*s %s\n", indent, "", -desc_margin + indent, cmd.name, cmd.desc);
        if (cmd.kind == HGL_CMD_NODE) {
            hgl_cmd_tree_print(cmd.sub_tree, indent + 4, desc_margin);
        }
        cmd = cmd_tree_root[i++];
    }
}

/*--- Private functions -----------------------------------------------------------------*/

static GapBuffer gb_make(char * const buf, size_t size)
{
    return (GapBuffer) {
        .data      = buf,
        .size      = size,
        .gap_start = 0,
        .gap_end   = size,
        .cursor    = 0,
    };
}

static void gb_clear(GapBuffer *gbuf)
{
    gbuf->gap_start = 0;
    gbuf->gap_end   = gbuf->size;
    gbuf->cursor    = 0;
}

static void gb_load_contents(GapBuffer *gbuf, const char *str, size_t length)
{
    gb_clear(gbuf);
    gb_insert_many(gbuf, str, length);
}

static char *gb_to_cstr(GapBuffer *gbuf, char *buf)
{
    int s = gbuf->gap_start;
    int e = gbuf->gap_end;
    int l = gb_length(gbuf);
    memcpy(buf, gbuf->data, s);
    memcpy(buf + s, &gbuf->data[e], l - s);
    buf[l] = '\0';
    return buf;
}

static int gb_length(GapBuffer *gbuf)
{
    return gbuf->size - (gbuf->gap_end - gbuf->gap_start);
}

static char gb_char_left_of_cursor(GapBuffer *gbuf)
{
    int s = gbuf->gap_start;
    int e = gbuf->gap_end;
    int c = gbuf->cursor - 1;
    int l = gb_length(gbuf);
    assert(c < l);
    if (c == -1) {
        return '\0'; // @Henrik TODO 2026-03-26 13:16:39: Think of something better to return?
    }

    if (c < gbuf->gap_start) {
        return gbuf->data[c];
    } else {
        return gbuf->data[c + (e - s)];
    }
}

static char gb_char_at_cursor(GapBuffer *gbuf)
{
    int s = gbuf->gap_start;
    int e = gbuf->gap_end;
    int c = gbuf->cursor;
    int l = gb_length(gbuf);
    if (c == l) {
        return '\0'; // @Henrik TODO 2026-03-26 14:25:14: Think of something better to return?
    }

    if (c < gbuf->gap_start) {
        return gbuf->data[c];
    } else {
        return gbuf->data[c + (e - s)];
    }
}

static void gb_position_split_on_cursor(GapBuffer *gbuf)
{
    int s = gbuf->gap_start;
    int e = gbuf->gap_end;
    int d = gbuf->cursor - s; 
    if (d < 0) { // L
        memcpy(&gbuf->data[e] + d, &gbuf->data[gbuf->cursor], -d);
    } else if (d > 0) { // R
        memcpy(&gbuf->data[s], &gbuf->data[e], d);
    }
    gbuf->gap_start = gbuf->cursor;
    gbuf->gap_end = e + d;
}

static void gb_insert_char(GapBuffer *gbuf, char c)
{
    int gblen = gb_length(gbuf);
    assert(gblen < gbuf->size); 
    gb_position_split_on_cursor(gbuf);
    gbuf->data[gbuf->cursor] = c;
    gbuf->gap_start++;
    gbuf->cursor++;
}

static void gb_insert_many(GapBuffer *gbuf, const char *str, int n)
{
    int gblen = gb_length(gbuf);
    assert((gblen + n) < gbuf->size); 
    gb_position_split_on_cursor(gbuf);
    memcpy(&gbuf->data[gbuf->cursor], str, n);
    gbuf->gap_start += n;
    gbuf->cursor += n;
}

static int gb_step_cursor(GapBuffer *gbuf, int d)
{
    int old_cursor = gbuf->cursor;
    gbuf->cursor += d;
    if (gbuf->cursor <= 0) gbuf->cursor = 0;
    if (gbuf->cursor > gb_length(gbuf)) gbuf->cursor = gb_length(gbuf);
    return gbuf->cursor - old_cursor;
}

static void gb_set_cursor(GapBuffer *gbuf, int i)
{
    gbuf->cursor = i;
    if (gbuf->cursor <= 0) gbuf->cursor = 0;
    if (gbuf->cursor > gb_length(gbuf)) gbuf->cursor = gb_length(gbuf);
}

static void gb_delete_right(GapBuffer *gbuf, int n)
{
    // @Henrik TODO 2026-03-25 23:21:01: Placeholder implementation. Make a more efficient solution.
    for (int i = 0; i < n; i++) {
        if (0 == gb_step_cursor(gbuf, 1)) {
            return;
        }
        gb_delete_left(gbuf, 1); 
    }
}

static void gb_delete_left(GapBuffer *gbuf, int n)
{
    if (gbuf->cursor == 0) {
        return;
    }
    gb_position_split_on_cursor(gbuf);
    n = (n > gbuf->gap_start) ? gbuf->gap_start : n;
    gbuf->gap_start -= n;
    gbuf->cursor -= n;
}

static void gb_delete_left_all(GapBuffer *gbuf)
{
    if (gbuf->cursor == 0) {
        return;
    }
    gb_position_split_on_cursor(gbuf);
    gbuf->gap_start = 0;
    gbuf->cursor = 0;
}

static void gb_delete_right_all(GapBuffer *gbuf)
{
    if (gbuf->cursor == gb_length(gbuf)) {
        return;
    }
    gb_position_split_on_cursor(gbuf);
    gbuf->gap_end = gbuf->size;
}


static int gb_word_under_cursor(GapBuffer *gbuf, char *buf)
{
    /* find start of word */
    int cursor = gbuf->cursor; // save cursor position
    while (!isspace(gb_char_left_of_cursor(gbuf))) {
        if (0 == gb_step_cursor(gbuf, -1)) {
            break;
        }
    }
    int word_start = gbuf->cursor;

    /* find end of word */
    gbuf->cursor = cursor; // restore cursor position
    while (!isspace(gb_char_at_cursor(gbuf))) {
        if (0 == gb_step_cursor(gbuf, 1)) {
            break;
        }
    }
    int word_end = gbuf->cursor;

    /* positon split at the end of the word and copy data */
    gb_position_split_on_cursor(gbuf);
    memcpy(buf, &gbuf->data[word_start], word_end - word_start);
    buf[word_end - word_start] = '\0';

    gbuf->cursor = cursor; // restore cursor position

    /* return cursor position relative to start of word */
    return gbuf->cursor - word_start;
}

// @Henrik TODO 2026-03-26 14:42:47: Fix ugly caret jumping artifacts
static void gb_display(GapBuffer *gbuf, const char *prompt)
{
    /* split on cursor for simplicity in printing */
    gb_position_split_on_cursor(gbuf);
        
    /* hide cursor (hides annoying flickering) */
    printf("\33[?25l");

    /* erase entire line */
    printf("\33[2K\r");

    /* print prompt + formatted gbuf contents */
    printf("%s", prompt);
    printf("%.*s", gbuf->gap_start, gbuf->data);
    printf("%.*s", (gbuf->size - gbuf->gap_end), &gbuf->data[gbuf->gap_end]);

    /* position caret*/
    int caret_pos = strlen(prompt) + gbuf->cursor;
    if (caret_pos != 0) {
        printf("\r\33[%dC", caret_pos);
    } else {
        printf("\r"); // apparently necessary.
    }

    /* show cursor again */
    printf("\33[?25h");

    fflush(stdout);
}

#if 0
static void gb_insert_cstr(GapBuffer *gbuf, const char *cstr)
{
    gb_insert_many(gbuf, cstr, (int)strlen(cstr));    
}
#endif

#if 0
static void gb_debug_print(GapBuffer *gbuf) 
{
    int c = gbuf->cursor;
    int s = gbuf->gap_start;
    int e = gbuf->gap_end;
    int l = e - s;
    printf("[");
    for (int i = 0; i < gbuf->size; i++) {
        if (c <= s) {
            if (i == c) printf("|");
        } else {
            if (i == c + l) printf("|");
        }
        if (i < s || i >= e) printf("%c,", gbuf->data[i]);
        else printf("_,");
    }
    printf("]  --->   \"");
    for (int i = 0; i < gbuf->size; i++) {
        if (c <= s) {
            if (i == c) printf("|");
        } else {
            if (i == c + l) printf("|");
        }
        if (i < s || i >= e) printf("%c", gbuf->data[i]);
    }
    printf("\"\n");
    printf("c = %d, s = %d, e = %d\n", gbuf->cursor, gbuf->gap_start, gbuf->gap_end);
}
#endif


static void mb_push(MatchBuffer *mbuf, const char *cstr)
{

    /* no backing buffer? */
    if (mbuf->arr == NULL) {
        mbuf->count = 0;
        mbuf->capacity = 32;
        mbuf->arr = HGL_CMD_ALLOC(mbuf->capacity * sizeof(*mbuf->arr));
        mbuf->longest_match = 0;
    }

    /* grow? */
    if (mbuf->capacity < (mbuf->count + 1)) {
        mbuf->capacity *= 2;
        mbuf->arr = HGL_CMD_REALLOC(mbuf->arr, mbuf->capacity * sizeof(*mbuf->arr));
    }

    /* push match */
    mbuf->arr[mbuf->count++] = cstr;

    /* update longest match */
    int cstr_len = strlen(cstr);
    if (cstr_len > mbuf->longest_match) {
        mbuf->longest_match = cstr_len; 
    }

    /* update lcp. Not necessary for the first match but idc. */
    if (mbuf->count == 1) {
        mbuf->longest_common_prefix = cstr_len;
    } else {
        int i;
        for (i = 0; i < mbuf->longest_common_prefix; i++) {
            if (mbuf->arr[0][i] != cstr[i]) {
                break;
            }
        }
        if (i < mbuf->longest_common_prefix) {
            mbuf->longest_common_prefix = i;
        }
    }
}

static void mb_clear(MatchBuffer *mbuf)
{
    mbuf->count                 = 0;
    mbuf->longest_match         = 0;
    mbuf->longest_common_prefix = -1;
}

static void mb_free(MatchBuffer *mbuf)
{
    HGL_CMD_FREE(mbuf->arr);
}

static void mb_display(MatchBuffer *mbuf)
{
    if (mbuf->count == 0) {
        return;
    }

    printf("\n");
    int cols = 80 / mbuf->longest_match;
    int rows = mbuf->count / cols;
    int rem  = mbuf->count % cols;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("  %*s", -mbuf->longest_match, mbuf->arr[cols*i + j]);
        }
        printf("\n");
    }
    if (rem != 0) { // TODO not necessary, right?
        for (int i = 0; i < rem; i++) {
            printf("  %*s", -mbuf->longest_match, mbuf->arr[cols*rows + i]);
        }
        printf("\n");
    }
}

static ComplBuffer cb_make(void *strs, int count, int stride)
{
    return (ComplBuffer) {
        .strs = strs,
        .count = count,
        .stride = stride,
    };
}

static ComplBuffer cb_make_from_cmd_tree(const HglCommand *cmd_tree)
{
    if (cmd_tree == NULL) {
        return (ComplBuffer) {
            .strs   = NULL,
            .count  = 0,
            .stride = 0,
        };
    }
    int i = 0;
    while (cmd_tree[i].kind != HGL_CMD_NONE) i++;
    return cb_make((void *)&cmd_tree[0].name, i, sizeof(HglCommand));
}

static void cb_match_prefix(ComplBuffer *cbuf, const char *prefix, MatchBuffer *matches)
{
    mb_clear(matches);

    if (prefix == NULL) {
        return;
    }

    uint8_t *strs8 = (uint8_t *) cbuf->strs;
    size_t prefix_len = strlen(prefix);
    for (int i = 0; i < cbuf->count; i++) {
        const char *candidate = *(const char **)(strs8 + i*cbuf->stride);
        size_t candidate_len = strlen(candidate);
        if (candidate_len < prefix_len) {
            continue;
        }
        if (strncmp(candidate, prefix, prefix_len) == 0) {
            mb_push(matches, candidate);
        }
    }

    return;
}

#if 0
static void cb_debug_print(ComplBuffer *cbuf)
{
    uint8_t *strs8 = (uint8_t *) cbuf->strs;
    for (int i = 0; i < cbuf->count; i++) {
        const char *s = *(const char **)(strs8 + i*cbuf->stride);
        printf(" - %s\n", s);
    }
}
#endif

static void hb_append(HistBuffer *hbuf, const char *str, size_t length)
{
    /* Empty? just blast entry into the buffer at index = 0 */
    if (hbuf->is_empty) {
        HistEntryHeader e = {
            .prev   = 0,
            .next   = 0,
            .length = length,
        };
        memcpy(hbuf->data, &e, sizeof(e));
        memcpy(hbuf->data + sizeof(e), str, length);
        hbuf->is_empty = false;
        hbuf->nav = HISTORY_PAST_END;
        return;
    }

    HistEntryHeader *curr  = NULL;
    HistEntryHeader *last  = (HistEntryHeader *)&hbuf->data[hbuf->last];
    HistEntryHeader *first = (HistEntryHeader *)&hbuf->data[hbuf->first];

    /* Equal to last entry? Skip */
    if ((last->length == length) && 
        (0 == memcmp(&hbuf->data[hbuf->last + sizeof(HistEntryHeader)], str, length))) {
        hbuf->nav = HISTORY_PAST_END;
        return;
    }

    /* round up to nearest multiple of 16 */
    uint32_t start = ((hbuf->last + sizeof(HistEntryHeader) + last->length) + 15) & (~0xF);
    uint32_t end = start + sizeof(HistEntryHeader) + length;

    assert((hbuf->last + sizeof(HistEntryHeader) + last->length) <= hbuf->size && "bug");

    /* wrap to 0 if necessary */
    bool wrap = false;
    if ((end >= hbuf->size) || (start >= hbuf->size)) {
        start = 0;
        end   = sizeof(HistEntryHeader) + length;
        wrap  = true;
    } 

    assert(end < hbuf->size && "entry too big 1");

    /* overwrite first entry if necessary */
    if (wrap || (hbuf->last < hbuf->first)) {
        wrap = false;
        while ((end > hbuf->first) && !wrap) {
            first = (HistEntryHeader *)&hbuf->data[hbuf->first];
            wrap = hbuf->first > first->next;
            hbuf->first = first->next;
        }
    }

    /* blast a new empty header + data into the buffer */
    HistEntryHeader e = {
        .prev   = 0,
        .next   = 0,
        .length = length,
    };
    memcpy(hbuf->data + start, &e, sizeof(e));
    memcpy(hbuf->data + start + sizeof(e), str, length);

    /* fix "pointers" */
    curr  = (HistEntryHeader *)&hbuf->data[start];
    last  = (HistEntryHeader *)&hbuf->data[hbuf->last];
    first = (HistEntryHeader *)&hbuf->data[hbuf->first];
    curr->next = hbuf->first;
    curr->prev = hbuf->last;
    first->prev = start;
    last->next = start;
    hbuf->last = start;

    /* Set nav to past end */
    hbuf->nav = HISTORY_PAST_END;
}

#if 0
static HistBuffer hb_make(void *buffer, size_t size)
{
    return (HistBuffer) {
        .data = buffer,
        .size = size,
        .first = 0u,
        .last  = 0u,
        .nav   = 0u,
        .is_empty = true,
    };
}
#endif

#if 0
static void hb_debug_print(const HistBuffer *hbuf)
{
    if (hbuf->is_empty) {
        printf("empty\n");
        return;
    }
    
    int first = hbuf->first;
    int curr  = first;
    while (true) {
        HistEntryHeader *e = (HistEntryHeader *)&hbuf->data[curr];
        char *str = (char*)&hbuf->data[curr + sizeof(HistEntryHeader)];
        int len = e->length;
        printf("\"%.*s\" --> ", len, str);
        curr = e->next;
        if (curr == first) {
            break;
        }
    }

    printf("\n");
}
#endif

//static const char *hb_navigate_prev(HistBuffer *hbuf, uint32_t *length)
//{
//    if (hbuf->is_empty) {
//        *length = 0;
//        return NULL;
//    }
//
//    HistEntryHeader *e = (HistEntryHeader *)&hbuf->data[hbuf->nav];
//    *length = e->length;
//    const char *str = (const char *)&hbuf->data[hbuf->nav + sizeof(HistEntryHeader)];
//
//    if (hbuf->nav != hbuf->first) {
//        hbuf->nav = e->prev;
//    }
//
//    return str;
//}
//
//static const char *hb_navigate_next(HistBuffer *hbuf, uint32_t *length)
//{
//    if (hbuf->is_empty) {
//        *length = 0;
//        return NULL;
//    }
//
//    HistEntryHeader *e = (HistEntryHeader *)&hbuf->data[hbuf->nav];
//    if (hbuf->nav != hbuf->last) {
//        hbuf->nav = e->next;
//    } else {
//        *length = 0;
//        return NULL;
//    }
//
//    e = (HistEntryHeader *)&hbuf->data[hbuf->nav];
//    *length = e->length;
//    const char *str = (const char *)&hbuf->data[hbuf->nav + sizeof(HistEntryHeader)];
//
//    return str;
//}

static const char *hb_navigate_prev(HistBuffer *hbuf, uint32_t *length)
{
    HistEntryHeader *e = NULL;

    if (hbuf->is_empty) {
        *length = 0;
        return NULL;
    }

    if (hbuf->nav == HISTORY_PAST_END) {
        hbuf->nav = hbuf->last;
    } else if (hbuf->nav != hbuf->first) {
        e = (HistEntryHeader *)&hbuf->data[hbuf->nav];
        hbuf->nav = e->prev;
    }

    e = (HistEntryHeader *)&hbuf->data[hbuf->nav];
    *length = e->length;
    return (const char *)&hbuf->data[hbuf->nav + sizeof(HistEntryHeader)];
}

static const char *hb_navigate_next(HistBuffer *hbuf, uint32_t *length)
{
    HistEntryHeader *e = NULL;

    if (hbuf->is_empty || hbuf->nav == HISTORY_PAST_END) {
        *length = 0;
        return NULL;
    }

    if (hbuf->nav == hbuf->last) {
        hbuf->nav = HISTORY_PAST_END;
        return NULL;
    } else {
        e = (HistEntryHeader *)&hbuf->data[hbuf->nav];
        hbuf->nav = e->next;
    }

    e = (HistEntryHeader *)&hbuf->data[hbuf->nav];
    *length = e->length;
    return (const char *)&hbuf->data[hbuf->nav + sizeof(HistEntryHeader)];
}

#endif /* HGL_CMD_IMPLEMENTATION */

