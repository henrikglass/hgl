
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
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
 * hgl_rbtree.h implements a red black tree.
 *
 *
 * USAGE:
 *
 * Include hgl_rbtree.h file like this:
 *
 *     #define HGL_RBTREE_IMPLEMENTATION
 *     #include "hgl_rbtree.h"
 *
 * You may also supply your own allocator to hgl_rbtree by redefining the
 * preprocessor variables before including:
 *
 *     HGL_RBTREE_ALLOC
 *     HGL_RBTREE_FREE
 *
 * example:
 *
 *     #define HGL_RBTREE_ALLOC my_pool_alloc
 *     #define HGL_RBTREE_FREE my_pool_free
 *     #define HGL_RBTREE_IMPLEMENTATION
 *     #include "hgl_rbtree.h"
 *
 *     // Compare functions for hgl_rbtree follow the same semantics
 *     // as those you'd pass to qsort.
 *     static int my_cmp(const void *a, const void *b)
 *     {
 *         int *a_ = (int *) a;
 *         int *b_ = (int *) b;
 *         return *a_ - *b_;
 *     }
 *
 *          /.../
 *
 *     int my_data[5] = {1, 2, 3, 4, 5};
 *     HglRbTree tree = hgl_rbtree_make(&my_cmp);
 *
 *     // insert data
 *     for (int i = 0; i < 5, i++) {
 *         hgl_rbtree_insert(&tree, (void *)&my_data[i]);
 *     }
 *
 *     // delete data
 *     int value = 3;
 *     hgl_rbtree_delete(&tree, &value);
 *
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_RBTREE_H
#define HGL_RBTREE_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdbool.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    HGL_RBTREE_RED,
    HGL_RBTREE_BLACK,
} HglRbTreeNodeColor;

typedef struct HglRbTreeNode
{
    void *item;
    HglRbTreeNodeColor color;
    struct HglRbTreeNode *parent;
    struct HglRbTreeNode *left;
    struct HglRbTreeNode *right;
} HglRbTreeNode;

typedef struct
{
    int (*cmp)(const void *, const void *);
    struct HglRbTreeNode *root;
    struct HglRbTreeNode *NIL;
} HglRbTree;

/*--- Public function prototypes --------------------------------------------------------*/

/**
 * Creates a new red-black tree and returns it. `cmp_func` is a compare function
 * used to define the order of items in the tree. It follows the same semantics
 * as the `compar` function passed to qsort (See "Man 3 qsort").
 */
HglRbTree hgl_rbtree_make(int (*cmp_func)(const void *, const void *));

/**
 * Inserts `item` into `tree`.
 */
int hgl_rbtree_insert(HglRbTree *tree, void *item);

/**
 * Inserts all items from tree `other` into `tree`. This effectively makes
 * `tree` the union of `tree` and `other`;
 */
void hgl_rbtree_join(HglRbTree *tree, HglRbTree *other);

/**
 * Deletes `item` from `tree`.
 */
void hgl_rbtree_delete(HglRbTree *tree, void *item);

/**
 * Returns true if `tree` contains `item`.
 */
bool hgl_rbtree_contains(HglRbTree *tree, void *item);

/**
 * Returns an item matching `item` if it exists in the tree, otherwise NULL.
 */
void *hgl_rbtree_lookup(HglRbTree *tree, void *item);

/**
 * Returns the smallest item in `tree`.
 */
void *hgl_rbtree_min(HglRbTree *tree);

/**
 * Returns the largest item in `tree`.
 */
void *hgl_rbtree_max(HglRbTree *tree);

/**
 * Returns the number of items in `tree`.
 */
size_t hgl_rbtree_count(HglRbTree *tree);

/**
 * Pretty-prints `tree`.
 */
void hgl_rbtree_print(HglRbTree *tree, HglRbTreeNode *node, int indent, int bheight);

/**
 * Destroys `tree` (free's all its memory). 
 */
void hgl_rbtree_destroy(HglRbTree *tree);

#endif /* HGL_RBTREE_H */

#ifdef HGL_RBTREE_IMPLEMENTATION

#include <stdint.h>
#include <assert.h>

#if !defined(HGL_RBTREE_ALLOC) &&   \
    !defined(HGL_RBTREE_FREE)
#include <stdlib.h>
#define HGL_RBTREE_ALLOC malloc
#define HGL_RBTREE_FREE free
#endif

#define HGL_RBTREE_ASSERT(x) (assert((x)))

static void hgl_rbtree_free_subtree_(HglRbTree *tree, HglRbTreeNode *node)
{
    if (node == tree->NIL) {
        return;
    }

    hgl_rbtree_free_subtree_(tree, node->left);
    hgl_rbtree_free_subtree_(tree, node->right);

    HGL_RBTREE_FREE(node);
}

static int hgl_rbtree_default_cmp_(const void *a, const void *b)
{
    size_t a_ = (size_t) a;
    size_t b_ = (size_t) b;
    if (a_ < b_) {
        return -1;
    } else if (a_ > b_) {
        return 1;
    } else {
        return 0;
    }
}

static HglRbTreeNode *hgl_rbtree_make_node_(HglRbTree *tree,
                                            HglRbTreeNode *parent,
                                            void *item,
                                            HglRbTreeNodeColor color)
{
    HglRbTreeNode *node = HGL_RBTREE_ALLOC(sizeof(HglRbTreeNode));
    if (node == NULL) {
        return NULL;
    }
    node->item   = item;
    node->color  = color;
    node->parent = parent;
    node->left   = tree->NIL;
    node->right  = tree->NIL;
    return node;
}

static void hgl_rbtree_rotr_in_place_(HglRbTree *tree, HglRbTreeNode *node)
{
    HGL_RBTREE_ASSERT(node->left != tree->NIL);
    void *temp_item;
    HglRbTreeNode *temp_node;

    /* swap items */
    temp_item        = node->item;
    node->item       = node->left->item;
    node->left->item = temp_item;

    /* swap left right */
    temp_node   = node->left;
    node->left  = node->right;
    node->right = temp_node;

    /* rotate subtrees clockwise */
    temp_node          = node->left;
    node->left         = node->right->left;
    node->right->left  = node->right->right;
    node->right->right = temp_node;

    /* update subtree parents */
    if (node->left != tree->NIL) {node->left->parent = node;}
    if (node->right->left != tree->NIL) {node->right->left->parent = node->right;} // not needed?
    if (node->right->right != tree->NIL) {node->right->right->parent = node->right;}
}

static void hgl_rbtree_rotl_in_place_(HglRbTree *tree, HglRbTreeNode *node)
{
    HGL_RBTREE_ASSERT(node->right != tree->NIL);
    void *temp_item;
    HglRbTreeNode *temp_node;

    /* swap items */
    temp_item         = node->item;
    node->item        = node->right->item;
    node->right->item = temp_item;

    /* swap left right */
    temp_node   = node->right;
    node->right = node->left;
    node->left  = temp_node;

    /* rotate subtrees counter-clockwise */
    temp_node         = node->right;
    node->right       = node->left->right;
    node->left->right = node->left->left;
    node->left->left  = temp_node;

    /* update subtree parents */
    if (node->right != tree->NIL) {node->right->parent = node;}
    if (node->left->right != tree->NIL) {node->left->right->parent = node->left;} // not needed?
    if (node->left->left != tree->NIL) {node->left->left->parent = node->left;}
}

static void hgl_rbtree_rotl_(HglRbTree *tree, HglRbTreeNode *x)
{
    HglRbTreeNode* y = x->right;
    x->right = y->left;

    if(y->left != tree->NIL) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if(x->parent == tree->NIL) {
        tree->root = y;
    } else if(x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

static void hgl_rbtree_rotr_(HglRbTree *tree, HglRbTreeNode *x)
{
    HglRbTreeNode* y = x->left;
    x->left = y->right;

    if(y->right != tree->NIL) {
        y->right->parent = x;
    }

    y->parent = x->parent;

    if(x->parent == tree->NIL) {
        tree->root = y;
    } else if(x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->right = x;
    x->parent = y;

}

static void hgl_rbtree_transplant_(HglRbTree *tree, HglRbTreeNode *u, HglRbTreeNode *v)
{
    if (u->parent == tree->NIL) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }

    v->parent = u->parent;
}

static void hgl_rbtree_join_inorder_insert_(HglRbTree *tree,
                                            HglRbTree *other,
                                            HglRbTreeNode *other_node)
{
    if (other_node == other->NIL) {
        return;
    }
    hgl_rbtree_join_inorder_insert_(tree, other, other_node->left);
    hgl_rbtree_insert(tree, other_node->item);
    hgl_rbtree_join_inorder_insert_(tree, other, other_node->right);
}

void hgl_rbtree_count_helper_(HglRbTree *tree, HglRbTreeNode *node, size_t *count)
{
    if (node == tree->NIL) {
        return;
    }
    (*count)++;
    hgl_rbtree_count_helper_(tree, node->left, count);
    hgl_rbtree_count_helper_(tree, node->right, count);
}


/*--- Public functions ------------------------------------------------------------------*/

HglRbTree hgl_rbtree_make(int (*cmp_func)(const void *, const void *))
{
    HglRbTree tree;

    tree.cmp = (cmp_func == NULL) ? hgl_rbtree_default_cmp_ : cmp_func,
    tree.NIL = HGL_RBTREE_ALLOC(sizeof(HglRbTreeNode));

    if (tree.NIL == NULL) {
        fprintf(stderr, "[hgl_rbtree_make] Error: buy more RAM lol.\n");
        return tree;
    }

    tree.root = tree.NIL;
    tree.NIL->item   = NULL;
    tree.NIL->color  = HGL_RBTREE_BLACK;
    tree.NIL->parent = tree.NIL;
    tree.NIL->left   = tree.NIL;
    tree.NIL->right  = tree.NIL;

    return tree;
}

int hgl_rbtree_insert(HglRbTree *tree, void *item)
{
    int cmp;
    HglRbTreeNode *n = tree->root;
    HglRbTreeNode *p = tree->NIL; /* n's parent*/
    HglRbTreeNode *gp;            /* n's grandparent */
    HglRbTreeNode *u;             /* n's uncle*/

    /* Find spot for insertion */
    while(n != tree->NIL) {
        cmp = tree->cmp(item, n->item);

        if (cmp == 0) {
            return 1;
        }

        p = n;
        n = (cmp < 0) ? n->left : n->right;
    }

    /* Special case: inserting root node. No violations, return immediately */
    if (p == tree->NIL) {
        tree->root = hgl_rbtree_make_node_(tree, tree->NIL, item, HGL_RBTREE_BLACK);
        return 0;
    }

    /* Insert red node */
    n = hgl_rbtree_make_node_(tree, p, item, HGL_RBTREE_RED);
    if (cmp < 0) {
        p->left = n;
    } else {
        p->right = n;
    }

    /* Black parent? no violations, return immediately */
    if (p->color == HGL_RBTREE_BLACK) {
        return 0;
    }

    /* Red parent: fix violations */
    while (n != tree->root && p->color == HGL_RBTREE_RED) {
        gp = p->parent;

        bool p_is_left = (p == gp->left);
        u = (p_is_left) ? gp->right : gp->left;

        /* Uncle is red => recolor */
        if (u->color == HGL_RBTREE_RED) {
            u->color  = HGL_RBTREE_BLACK;
            p->color  = HGL_RBTREE_BLACK;
            gp->color = HGL_RBTREE_RED;

            n = gp;
            p = n->parent;
            continue;
        }

        /* Uncle is black => perform rotations */
        if (p_is_left) {
            if (n == p->right){
                hgl_rbtree_rotl_in_place_(tree, p);
            }
            hgl_rbtree_rotr_in_place_(tree, gp);
        } else {
            if (n == p->left){
                hgl_rbtree_rotr_in_place_(tree, p);
            }
            hgl_rbtree_rotl_in_place_(tree, gp);
        }

        return 0;
    }

    if (n == tree->root) {
        n->color = HGL_RBTREE_BLACK;
    }

    return 0;
}

void hgl_rbtree_join(HglRbTree *tree, HglRbTree *other)
{
    HGL_RBTREE_ASSERT(tree != NULL);
    HGL_RBTREE_ASSERT(other != NULL);
    hgl_rbtree_join_inorder_insert_(tree, other, other->root);
}

void hgl_rbtree_delete(HglRbTree *tree, void *item)
{
    HglRbTreeNode *x;
    HglRbTreeNode *y;
    HglRbTreeNode *z;
    HglRbTreeNode *w;

    /* lookup item */
    z = tree->root;
    while (z != tree->NIL) {
        int cmp = tree->cmp(item, z->item);
        if (cmp == 0) {
            break;
        }
        z = (cmp < 0) ? z->left : z->right;
    }

    /* item not found => return immediately */
    if (z == tree->NIL) {
        return;
    }

    /* make note of original color */
    HglRbTreeNodeColor orig_color = z->color;

    /* Delete node */
    if (z->left == tree->NIL) {
        x = z->right;
        hgl_rbtree_transplant_(tree, z, z->right);
    } else if (z->right == tree->NIL) {
        x = z->left;
        hgl_rbtree_transplant_(tree, z, z->left);
    } else {
        y = z->right; while (y->left != tree->NIL) y = y->left;
        x = y->right;
        orig_color = y->color;

        if (y->parent == z) {
            x->parent = y;
        } else {
            hgl_rbtree_transplant_(tree, y, y->right);
            y->right = z->right;
            z->right->parent = y;
        }

        hgl_rbtree_transplant_(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    HGL_RBTREE_FREE(z);

    /* Original color was red => red black properties are intact */
    if (orig_color == HGL_RBTREE_RED) {
        return;
    }

    /* Fix red-black properties */
    while (x != tree->root && x->color == HGL_RBTREE_BLACK) {
        bool is_left = (x == x->parent->left);
        w = (is_left) ? x->parent->right : x->parent->left;

        if (w->color == HGL_RBTREE_RED) {
            w->color = HGL_RBTREE_BLACK;
            x->parent->color = HGL_RBTREE_RED;
            if (is_left) {
                hgl_rbtree_rotl_(tree, x->parent);
                w = x->parent->right;
            } else {
                hgl_rbtree_rotr_(tree, x->parent);
                w = x->parent->left;
            }
        }

        if ((w->left->color == HGL_RBTREE_BLACK) &&
            (w->right->color == HGL_RBTREE_BLACK)) {
            w->color = HGL_RBTREE_RED;
            x = x->parent;
            continue;
        }

        if (is_left && w->right->color == HGL_RBTREE_BLACK) {
            w->left->color = HGL_RBTREE_BLACK;
            w->color = HGL_RBTREE_RED;
            hgl_rbtree_rotr_(tree, w);
            w = x->parent->right;
        }

        if (!is_left && w->left->color == HGL_RBTREE_BLACK) {
            w->right->color = HGL_RBTREE_BLACK;
            w->color = HGL_RBTREE_RED;
            hgl_rbtree_rotl_(tree, w);
            w = x->parent->left;
        }

        w->color = x->parent->color;
        x->parent->color = HGL_RBTREE_BLACK;

        if (is_left) {
            w->right->color = HGL_RBTREE_BLACK;
            hgl_rbtree_rotl_(tree, x->parent);
        } else {
            w->left->color = HGL_RBTREE_BLACK;
            hgl_rbtree_rotr_(tree, x->parent);
        }
        break;
    }

    x->color = HGL_RBTREE_BLACK;
}

bool hgl_rbtree_contains(HglRbTree *tree, void *item)
{
    return hgl_rbtree_lookup(tree, item) != NULL;
}

void *hgl_rbtree_lookup(HglRbTree *tree, void *item)
{
    HglRbTreeNode *n = tree->root;
    while(n != tree->NIL) {
        int cmp = tree->cmp(item, n->item);
        if (cmp == 0) {
            return n->item;
        }
        n = (cmp < 0) ? n->left : n->right;
    }
    return NULL;
}

void *hgl_rbtree_min(HglRbTree *tree)
{
    HglRbTreeNode *n = tree->root;
    while (n->left != tree->NIL) {
        n = n->left;
    }
    return n->item;
}

void *hgl_rbtree_max(HglRbTree *tree)
{
    HglRbTreeNode *n = tree->root;
    while (n->right != tree->NIL) {
        n = n->right;
    }
    return n->item;
}

size_t hgl_rbtree_count(HglRbTree *tree)
{
    size_t count = 0;
    hgl_rbtree_count_helper_(tree, tree->root, &count);
    return count;
}

void hgl_rbtree_print(HglRbTree *tree, HglRbTreeNode *node, int indent, int bheight)
{
    if (node == tree->NIL) {
        printf("%*s>NIL (B) @%d\n", indent, " ", bheight + 1);
    } else {
        printf("%*s>%d (%s)\n", indent, " ", *(int*)node->item, (node->color == HGL_RBTREE_BLACK) ? "B" : "R");
        hgl_rbtree_print(tree, node->left, indent + 2, (node->color == HGL_RBTREE_BLACK) ? bheight + 1 : bheight);
        hgl_rbtree_print(tree, node->right, indent + 2, (node->color == HGL_RBTREE_BLACK) ? bheight + 1 : bheight);
    }
}

void hgl_rbtree_destroy(HglRbTree *tree)
{
    hgl_rbtree_free_subtree_(tree, tree->root);
    HGL_RBTREE_FREE(tree->NIL);
}

#endif

