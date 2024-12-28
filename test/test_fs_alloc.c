
#include "hgl_test.h"

#define HGL_FS_ALLOC_ALIGNMENT 128
#define HGL_FS_ALLOC_IMPLEMENTATION
#include "hgl_fs_alloc.h"

static HglFsAllocator fs_allocator;

GLOBAL_SETUP {
    fs_allocator = hgl_fs_make(64*1024, 16);
}

GLOBAL_TEARDOWN {
    hgl_fs_destroy(&fs_allocator);
}

TEST(test_simple_alloc) {
    void *p0 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p1 = hgl_fs_alloc(&fs_allocator, 1000); 
    void *p2 = hgl_fs_alloc(&fs_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_fs_alloc(&fs_allocator, 0) == NULL);
}

TEST(test_free_all_behavior) {
    void *p0 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p1 = hgl_fs_alloc(&fs_allocator, 1000); 
    void *p2 = hgl_fs_alloc(&fs_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_fs_free_all(&fs_allocator);

    void *p3 = hgl_fs_alloc(&fs_allocator, 100);

    ASSERT(p0 == p3);
}

TEST(test_free_single_1) {
    void *p0 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p1 = hgl_fs_alloc(&fs_allocator, 1000); 
    void *p2 = hgl_fs_alloc(&fs_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_fs_free(&fs_allocator, p0);

    void *p3 = hgl_fs_alloc(&fs_allocator, 500);

    ASSERT(p0 != p3);

    void *p4 = hgl_fs_alloc(&fs_allocator, 50);

    ASSERT(p0 == p4);

}

TEST(test_free_single_2) {
    void *p0 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p1 = hgl_fs_alloc(&fs_allocator, 1000); 
    void *p2 = hgl_fs_alloc(&fs_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_fs_free(&fs_allocator, p1);

    void *p3 = hgl_fs_alloc(&fs_allocator, 5000);

    ASSERT(p1 != p3);
    ASSERT(p0 != p3);

    void *p4 = hgl_fs_alloc(&fs_allocator, 500);

    ASSERT(p1 == p4);
}

TEST(test_free_multiple) {
    void *p0 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p1 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p2 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p3 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p4 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p5 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p6 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p7 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p8 = hgl_fs_alloc(&fs_allocator, 100); 
    void *p9 = hgl_fs_alloc(&fs_allocator, 100); 

    hgl_fs_free(&fs_allocator, p1);
    hgl_fs_free(&fs_allocator, p3);
    hgl_fs_free(&fs_allocator, p5);
    hgl_fs_free(&fs_allocator, p7);
    hgl_fs_free(&fs_allocator, p9);

    ASSERT(hgl_fs_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_fs_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_fs_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_fs_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_fs_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_fs_alloc(&fs_allocator, 100) > p9);
}

TEST(test_too_big_alloc) {
    void *p0 = hgl_fs_alloc(&fs_allocator, 32*1024); 
    ASSERT(p0 != NULL);
    void *p1 = hgl_fs_alloc(&fs_allocator, 24*1024); 
    ASSERT(p1 != NULL);
    void *p2 = hgl_fs_alloc(&fs_allocator, 24*1024); 
    ASSERT(p2 == NULL);
}
