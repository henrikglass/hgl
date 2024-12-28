#include <stdio.h>

#define HGL_MEMDBG_IMPLEMENTATION
#include "hgl_memdbg.h"

#define HGL_POOL_ALLOC_IMPLEMENTATION
#include "hgl_pool_alloc.h"

static HglPool pool;

void *pool_alloc(size_t size)
{
    assert(size == pool.chunk_size);
    return hgl_pool_alloc(&pool);
}

void pool_free(void *ptr)
{
    hgl_pool_free(&pool, ptr);
}

void pool_free_all()
{
    hgl_pool_free_all(&pool);
}

#define HGL_RBTREE_ALLOC pool_alloc
#define HGL_RBTREE_FREE pool_free
#define HGL_RBTREE_IMPLEMENTATION
#include "hgl_rbtree.h"

static void verify_rb_properties_3_and_5(HglRbTree *tree, HglRbTreeNode *node)
{
    if (node == tree->NIL) {
        return;
    }
    if (node->color == HGL_RBTREE_RED) {
        assert(node->left->color == HGL_RBTREE_BLACK);
        assert(node->right->color == HGL_RBTREE_BLACK);
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

    assert(bhl == bhr);

    return bhl;
}

static void verify_rb_order(HglRbTree *tree, HglRbTreeNode *node)
{
    if (node->left != tree->NIL) {
        assert(tree->cmp(node->left->item, node->item) < 0);
        verify_rb_order(tree, node->left);
    }
    if (node->right != tree->NIL) {
        assert(tree->cmp(node->right->item, node->item) > 0);
        verify_rb_order(tree, node->right);
    }
}

void verify_rb_properties(HglRbTree *tree)
{
    /* Property #0 => all nodes are red or black. Always OK! */

    /* Property #1 => root is black */
    assert(tree->root->color == HGL_RBTREE_BLACK);

    /* Property #3 => red nodes must have black children */
    /* Property #5 => NIL nodes are black */
    verify_rb_properties_3_and_5(tree, tree->root);

    /* Property #4 => red nodes must have black children */
    verify_rb_property_4(tree, tree->root, 0);

    /* verify order */
    verify_rb_order(tree, tree->root);

    printf("Tree is a valid red-black tree!\n");
}

static int my_cmp(const void *a, const void *b)
{
    int *a_ = (int *) a;
    int *b_ = (int *) b;
    return *a_ - *b_;
}

int main(void)
{
    pool = hgl_pool_make(512, sizeof(HglRbTreeNode));

    int my_ints[] = {1,5,2,7,6,4,1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,1,3};

    HglRbTree rbt = hgl_rbtree_make(&my_cmp);
    HglRbTree *tree = &rbt;

    verify_rb_properties(tree);

    for (size_t i = 0; i < (sizeof(my_ints) / sizeof(my_ints[0])); i++) {
        hgl_rbtree_insert(tree, (void *) &my_ints[i]);
        verify_rb_properties(tree);
    }
        
    int j = 10;
    hgl_rbtree_insert(tree, (void *) &j);
    hgl_rbtree_insert(tree, (void *) &j);
    hgl_rbtree_insert(tree, (void *) &j);
    hgl_rbtree_insert(tree, (void *) &j);
    hgl_rbtree_insert(tree, (void *) &j);


    for (int i = 0; i < 40; i++) {
        printf("looking up %d: ", i);
        if (hgl_rbtree_lookup(tree, &i) != NULL) {
            printf("FOUND!\n");
        } else {
            printf("NOT FOUND!\n");
        }
    }

    printf("pool chunks used: %ld\n", pool.n_chunks - pool.free_stack_head - 1);

    verify_rb_properties(tree);

    hgl_rbtree_print(tree, tree->root, 0, 0);
    int i = 16;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 22;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 16;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 1;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 5;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 20;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 12;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 6;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 4;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 21;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 23;
    hgl_rbtree_delete(tree, &i);
    //hgl_rbtree_print(tree, tree->root, 0, 0);
    verify_rb_properties(tree);
    i = 25;
    hgl_rbtree_delete(tree, &i);
    //hgl_rbtree_print(tree, tree->root, 0, 0);
    verify_rb_properties(tree);
    i = 13;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 18;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 10;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 11;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 17;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    i = 19;
    hgl_rbtree_delete(tree, &i);
    verify_rb_properties(tree);
    hgl_rbtree_print(tree, tree->root, 0, 0);

    printf("Joining (union) with self: \n");
    hgl_rbtree_join(tree, tree);
    hgl_rbtree_print(tree, tree->root, 0, 0);

    printf("tree count: %zu\n", hgl_rbtree_count(tree));
    printf("pool chunks used: %ld\n", pool.n_chunks - pool.free_stack_head - 1);
    printf("Note: One extra chunk used for (dummy) NIL node\n");

    printf("Destroying tree:\n");
    hgl_rbtree_destroy(tree);
    printf("pool chunks used: %ld\n", pool.n_chunks - pool.free_stack_head - 1);


    pool_free_all();
    hgl_pool_destroy(&pool);

    hgl_memdbg_report();

    return 0;
}
