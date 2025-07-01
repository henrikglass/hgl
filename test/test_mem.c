
#include "hgl_test.h"

#include "hgl_int.h"
#include "hgl_mem.h"

TEST(test_memset)
{
    u16 a[4];
    u32 b[4];
    u64 c[4];

    memset16(a, 0xBEEF, 4);
    memset32(b, 0xDEADBEEF, 4);
    memset64(c, 0xCAFED00DBABEBEEF, 4);

    ASSERT(a[3] = 0xBEEF);
    ASSERT(b[4] = 0xDEADBEEF);
    ASSERT(c[4] = 0xCAFED00DBABEBEEF);
}

TEST(test_memcpy)
{
    u32 a[4];
    u32 b[4];

    memset32(b, 0xDEADBEEF, 4);
    memcpy32(a, b, 4);

    ASSERT(a[0] == 0xDEADBEEF);
    ASSERT(a[1] == 0xDEADBEEF);
    ASSERT(a[2] == 0xDEADBEEF);
    ASSERT(a[3] == 0xDEADBEEF);
}

TEST(test_rw_endian)
{
    u8 a[] = {
        0x00, 0x11, 0x22, 0x33, 
        0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB,
        0xCC, 0xDD, 0xEE, 0xFF,
    };

    ASSERT(read32le((void *)a) == 0x33221100); 
    ASSERT(read32be((void *)a) == 0x00112233); 
    ASSERT(read32le_unaligned((void *)(a + 1)) == 0x44332211); 
    ASSERT(read32be_unaligned((void *)(a + 1)) == 0x11223344); 

    u16 *ptr;

    ptr = (u16 *)(a + 2);
    write16le(ptr, 0xABCD);
    ASSERT(read16le(ptr) == 0xABCD); 

    ptr = (u16 *)(a + 3);
    write16le_unaligned(ptr, 0xABCD);
    ASSERT(read16le_unaligned(ptr) == 0xABCD); 
    ASSERT(read16be_unaligned(ptr) == 0xCDAB); 
}

TEST(test_rw_endian_unintended_unaligned_access, .expect_signal = SIGABRT)
{
    u8 a[] = {
        0x00, 0x11, 0x22, 0x33, 
        0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB,
        0xCC, 0xDD, 0xEE, 0xFF,
    };

    u16 *ptr;
    ptr = (u16 *)(a + 3);
    read16le(ptr); // unaligned access -- fail unless NDEBUG defined
    ASSERT(false); // we should never reach this
}

TEST(test_memfrobn)
{
    const char *str = "My Secret";
    char *cpy = strdup(str);
    uint64_t key = 0xA71BB490F88DC1B5;
    memfrobn(cpy, strlen(cpy) - 1, &key, sizeof(key));
    ASSERT_CSTR_NEQ(cpy, str);
    memfrobn(cpy, strlen(cpy) - 1, &key, sizeof(key));
    ASSERT_CSTR_EQ(cpy, str);
}
