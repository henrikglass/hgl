
#include "hgl_test.h"

#define HGL_RBTREE_IMPLEMENTATION
#include "hgl_rbtree.h"

static inline int my_str_cmp(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

static inline int my_int_cmp(const void *a, const void *b)
{
    int ai = *(int *) a;
    int bi = *(int *) b;
    return ai - bi;
}

static HglRbTree str_tree;
static HglRbTree int_tree;

static void verify_rb_properties_3_and_5(HglRbTree *tree, HglRbTreeNode *node)
{
    if (node == tree->NIL) {
        return;
    }
    if (node->color == HGL_RBTREE_RED) {
        ASSERT(node->left->color == HGL_RBTREE_BLACK);
        ASSERT(node->right->color == HGL_RBTREE_BLACK);
    }
    verify_rb_properties_3_and_5(tree, node->left);
    verify_rb_properties_3_and_5(tree, node->right);
}

static int verify_rb_property_4(HglRbTree *tree, HglRbTreeNode *node, int bh)
{
    if (node == tree->NIL) {
        return bh + 1;
    }

    if (node->color == HGL_RBTREE_BLACK) {
        bh++;
    }

    int bhl = verify_rb_property_4(tree, node->left, bh);
    int bhr = verify_rb_property_4(tree, node->right, bh);

    ASSERT(bhl == bhr);

    return bhl;
}

static void verify_rb_order(HglRbTree *tree, HglRbTreeNode *node)
{
    if (node->left != tree->NIL) {
        ASSERT(tree->cmp(node->left->item, node->item) < 0);
        verify_rb_order(tree, node->left);
    }
    if (node->right != tree->NIL) {
        ASSERT(tree->cmp(node->right->item, node->item) > 0);
        verify_rb_order(tree, node->right);
    }
}

void verify_rb_properties(HglRbTree *tree)
{
    /* Property #0 => all nodes are red or black. Always OK! */

    /* Property #1 => root is black */
    ASSERT(tree->root->color == HGL_RBTREE_BLACK);

    /* Property #3 => red nodes must have black children */
    /* Property #5 => NIL nodes are black */
    verify_rb_properties_3_and_5(tree, tree->root);

    /* Property #4 => red nodes must have black children */
    verify_rb_property_4(tree, tree->root, 0);

    /* verify order */
    verify_rb_order(tree, tree->root);
}

GLOBAL_SETUP
{
    str_tree = hgl_rbtree_make(my_str_cmp);
    int_tree = hgl_rbtree_make(my_int_cmp);
}

TEST(test_insert_remove_lookup)
{
    char *lenny = "Lenny";

    hgl_rbtree_insert(&str_tree, "Homer");
    hgl_rbtree_insert(&str_tree, "Marge");
    hgl_rbtree_insert(&str_tree, "Lisa");
    hgl_rbtree_insert(&str_tree, lenny);
    hgl_rbtree_insert(&str_tree, "Lenny"); // insert twice has no effect, given the semantics of `my_str_cmp`.
    hgl_rbtree_insert(&str_tree, "Bart");
    hgl_rbtree_insert(&str_tree, "Maggie");

    ASSERT(6 == hgl_rbtree_count(&str_tree));

    ASSERT(hgl_rbtree_contains(&str_tree, lenny));
    ASSERT(lenny == hgl_rbtree_lookup(&str_tree, lenny));

    hgl_rbtree_delete(&str_tree, "Lenny");

    ASSERT(!hgl_rbtree_contains(&str_tree, lenny));
    ASSERT(lenny != hgl_rbtree_lookup(&str_tree, lenny));

    verify_rb_properties(&str_tree);
}

TEST(test_insert_order)
{
    int data[] = {4,1,13,2,8,3,4,9,11};

    for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
        hgl_rbtree_insert(&int_tree, &data[i]);
    }

    ASSERT(13 == *(int*)hgl_rbtree_max(&int_tree));

    verify_rb_properties(&int_tree);
}

TEST(test_join)
{
    int data0[] = {0,2,4,6,8};
    int data1[] = {1,3,5,7,9};

    HglRbTree int_tree2 = hgl_rbtree_make(my_int_cmp);
    for (size_t i = 0; i < sizeof(data0)/sizeof(data0[0]); i++) {
        hgl_rbtree_insert(&int_tree, &data0[i]);
        hgl_rbtree_insert(&int_tree2, &data1[i]);
    }

    verify_rb_properties(&int_tree);
    verify_rb_properties(&int_tree2);

    hgl_rbtree_join(&int_tree, &int_tree2);

    verify_rb_properties(&int_tree);

    ASSERT(0 == *(int*)hgl_rbtree_min(&int_tree));
    ASSERT(9 == *(int*)hgl_rbtree_max(&int_tree));

    hgl_rbtree_destroy(&int_tree2);
}
