#ifndef HGL_MEM_H
#define HGL_MEM_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>

/*--- Public macros ---------------------------------------------------------------------*/

#define GCC_COMPILER (defined(__GNUC__) && !defined(__clang__))

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

static inline void memset16(uint16_t *ptr, uint16_t v, size_t n);
static inline void memset32(uint32_t *ptr, uint32_t v, size_t n);
static inline void memset64(uint64_t *ptr, uint64_t v, size_t n);

static inline void memcpy16(uint16_t *restrict dst, const uint16_t *restrict src, size_t n);
static inline void memcpy32(uint32_t *restrict dst, const uint32_t *restrict src, size_t n);
static inline void memcpy64(uint64_t *restrict dst, const uint64_t *restrict src, size_t n);

static inline uint16_t read16le(const uint16_t *ptr);
static inline uint32_t read32le(const uint32_t *ptr);
static inline uint64_t read64le(const uint64_t *ptr);
static inline uint16_t read16le_unaligned(const void *ptr);
static inline uint32_t read32le_unaligned(const void *ptr);
static inline uint64_t read64le_unaligned(const void *ptr);

static inline uint16_t read16be(const uint16_t *ptr);
static inline uint32_t read32be(const uint32_t *ptr);
static inline uint64_t read64be(const uint64_t *ptr);
static inline uint16_t read16be_unaligned(const void *ptr);
static inline uint32_t read32be_unaligned(const void *ptr);
static inline uint64_t read64be_unaligned(const void *ptr);

static inline void write16le(uint16_t *ptr, uint16_t v);
static inline void write32le(uint32_t *ptr, uint32_t v);
static inline void write64le(uint64_t *ptr, uint64_t v);
static inline void write16le_unaligned(void *ptr, uint16_t v);
static inline void write32le_unaligned(void *ptr, uint32_t v);
static inline void write64le_unaligned(void *ptr, uint64_t v);

static inline void write16be(uint16_t *ptr, uint16_t v);
static inline void write32be(uint32_t *ptr, uint32_t v);
static inline void write64be(uint64_t *ptr, uint64_t v);
static inline void write16be_unaligned(void *ptr, uint16_t v);
static inline void write32be_unaligned(void *ptr, uint32_t v);
static inline void write64be_unaligned(void *ptr, uint64_t v);

static inline void memfrobn(void *ptr, size_t n, void *key, size_t keylen);

static inline void memset16(uint16_t *ptr, uint16_t v, size_t n)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 1) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 16);
#endif
    uint16_t *ptr16 = (uint16_t *)ptr8;
    for (size_t i = 0; i < n; i++) {
        ptr16[i] = v;
    }
}

static inline void memset32(uint32_t *ptr, uint32_t v, size_t n)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 3) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 32);
#endif
    uint32_t *ptr32 = (uint32_t *)ptr8;
    for (size_t i = 0; i < n; i++) {
        ptr32[i] = v;
    }
}

static inline void memset64(uint64_t *ptr, uint64_t v, size_t n)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 7) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 64);
#endif
    uint64_t *ptr64 = (uint64_t *)ptr8;
    for (size_t i = 0; i < n; i++) {
        ptr64[i] = v;
    }
}

static inline void memcpy16(uint16_t *restrict dst, const uint16_t *restrict src, size_t n)
{
    assert((((uintptr_t)src) & 1) == 0);
    assert((((uintptr_t)dst) & 1) == 0);
#ifdef GCC_COMPILER
    src = __builtin_assume_aligned(src, 16);
    dst = __builtin_assume_aligned(dst, 16);
#endif
    const uint16_t *src16 = (const uint16_t *)src;
    uint16_t *dst16 = (uint16_t *)dst;
    for (size_t i = 0; i < n; i++) {
        *dst16++ = *src16++;
    }
}

static inline void memcpy32(uint32_t *restrict dst, const uint32_t *restrict src, size_t n)
{
    assert((((uintptr_t)src) & 3) == 0);
    assert((((uintptr_t)dst) & 3) == 0);
#ifdef GCC_COMPILER
    src = __builtin_assume_aligned(src, 32);
    dst = __builtin_assume_aligned(dst, 32);
#endif
    const uint32_t *src32 = (const uint32_t *)src;
    uint32_t *dst32 = (uint32_t *)dst;
    for (size_t i = 0; i < n; i++) {
        *dst32++ = *src32++;
    }
}

static inline void memcpy64(uint64_t *restrict dst, const uint64_t *restrict src, size_t n)
{
    assert((((uintptr_t)src) & 7) == 0);
    assert((((uintptr_t)dst) & 7) == 0);
#ifdef GCC_COMPILER
    src = __builtin_assume_aligned(src, 64);
    dst = __builtin_assume_aligned(dst, 64);
#endif
    const uint64_t *src64 = (const uint64_t *)src;
    uint64_t *dst64 = (uint64_t *)dst;
    for (size_t i = 0; i < n; i++) {
        *dst64++ = *src64++;
    }
}

static inline uint16_t read16le(const uint16_t *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 1) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 16);
#endif
    return ((uint16_t)ptr8[0] <<  0) |
           ((uint16_t)ptr8[1] <<  8);
}

static inline uint32_t read32le(const uint32_t *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 3) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 32);
#endif
    return ((uint32_t)ptr8[0] <<  0) |
           ((uint32_t)ptr8[1] <<  8) |
           ((uint32_t)ptr8[2] << 16) |
           ((uint32_t)ptr8[3] << 24);
}

static inline uint64_t read64le(const uint64_t *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 7) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 64);
#endif
    return ((uint64_t)ptr8[0] <<  0) |
           ((uint64_t)ptr8[1] <<  8) |
           ((uint64_t)ptr8[2] << 16) |
           ((uint64_t)ptr8[3] << 24) |
           ((uint64_t)ptr8[4] << 32) |
           ((uint64_t)ptr8[5] << 40) |
           ((uint64_t)ptr8[6] << 48) |
           ((uint64_t)ptr8[7] << 56);
}

static inline uint16_t read16le_unaligned(const void *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    return ((uint16_t)ptr8[0] <<  0) |
           ((uint16_t)ptr8[1] <<  8);
}

static inline uint32_t read32le_unaligned(const void *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    return ((uint32_t)ptr8[0] <<  0) |
           ((uint32_t)ptr8[1] <<  8) |
           ((uint32_t)ptr8[2] << 16) |
           ((uint32_t)ptr8[3] << 24);
}

static inline uint64_t read64le_unaligned(const void *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    return ((uint64_t)ptr8[0] <<  0) |
           ((uint64_t)ptr8[1] <<  8) |
           ((uint64_t)ptr8[2] << 16) |
           ((uint64_t)ptr8[3] << 24) |
           ((uint64_t)ptr8[4] << 32) |
           ((uint64_t)ptr8[5] << 40) |
           ((uint64_t)ptr8[6] << 48) |
           ((uint64_t)ptr8[7] << 56);
}

static inline uint16_t read16be(const uint16_t *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 1) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 16);
#endif
    return ((uint16_t)ptr8[0] <<  8) |
           ((uint16_t)ptr8[1] <<  0);
}

static inline uint32_t read32be(const uint32_t *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 3) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 32);
#endif
    return ((uint32_t)ptr8[0] << 24) |
           ((uint32_t)ptr8[1] << 16) |
           ((uint32_t)ptr8[2] <<  8) |
           ((uint32_t)ptr8[3] <<  0);
}

static inline uint64_t read64be(const uint64_t *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 7) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 64);
#endif
    return ((uint64_t)ptr8[0] << 56) |
           ((uint64_t)ptr8[1] << 48) |
           ((uint64_t)ptr8[2] << 40) |
           ((uint64_t)ptr8[3] << 32) |
           ((uint64_t)ptr8[4] << 24) |
           ((uint64_t)ptr8[5] << 16) |
           ((uint64_t)ptr8[6] <<  8) |
           ((uint64_t)ptr8[7] <<  0);
}

static inline uint16_t read16be_unaligned(const void *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    return ((uint16_t)ptr8[0] <<  8) |
           ((uint16_t)ptr8[1] <<  0);
}

static inline uint32_t read32be_unaligned(const void *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    return ((uint32_t)ptr8[0] << 24) |
           ((uint32_t)ptr8[1] << 16) |
           ((uint32_t)ptr8[2] <<  8) |
           ((uint32_t)ptr8[3] <<  0);
}

static inline uint64_t read64be_unaligned(const void *ptr)
{
    const uint8_t *ptr8 = (const uint8_t *)ptr;
    return ((uint64_t)ptr8[0] << 56) |
           ((uint64_t)ptr8[1] << 48) |
           ((uint64_t)ptr8[2] << 40) |
           ((uint64_t)ptr8[3] << 32) |
           ((uint64_t)ptr8[4] << 24) |
           ((uint64_t)ptr8[5] << 16) |
           ((uint64_t)ptr8[6] <<  8) |
           ((uint64_t)ptr8[7] <<  0);
}

static inline void write16le(uint16_t *ptr, uint16_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 1) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 16);
#endif
    ptr8[0] = (uint16_t)((v >> 0) & 0xFF);
    ptr8[1] = (uint16_t)((v >> 8) & 0xFF);
}

static inline void write32le(uint32_t *ptr, uint32_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 3) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 32);
#endif
    ptr8[0] = (uint16_t)((v >>  0) & 0xFF);
    ptr8[1] = (uint16_t)((v >>  8) & 0xFF);
    ptr8[2] = (uint16_t)((v >> 16) & 0xFF);
    ptr8[3] = (uint16_t)((v >> 24) & 0xFF);
}

static inline void write64le(uint64_t *ptr, uint64_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 7) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 64);
#endif
    ptr8[0] = (uint16_t)((v >>  0) & 0xFF);
    ptr8[1] = (uint16_t)((v >>  8) & 0xFF);
    ptr8[2] = (uint16_t)((v >> 16) & 0xFF);
    ptr8[3] = (uint16_t)((v >> 24) & 0xFF);
    ptr8[4] = (uint16_t)((v >> 32) & 0xFF);
    ptr8[5] = (uint16_t)((v >> 40) & 0xFF);
    ptr8[6] = (uint16_t)((v >> 48) & 0xFF);
    ptr8[7] = (uint16_t)((v >> 56) & 0xFF);
}

static inline void write16le_unaligned(void *ptr, uint16_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    ptr8[0] = (uint16_t)((v >> 0) & 0xFF);
    ptr8[1] = (uint16_t)((v >> 8) & 0xFF);
}

static inline void write32le_unaligned(void *ptr, uint32_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    ptr8[0] = (uint16_t)((v >>  0) & 0xFF);
    ptr8[1] = (uint16_t)((v >>  8) & 0xFF);
    ptr8[2] = (uint16_t)((v >> 16) & 0xFF);
    ptr8[3] = (uint16_t)((v >> 24) & 0xFF);
}

static inline void write64le_unaligned(void *ptr, uint64_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    ptr8[0] = (uint16_t)((v >>  0) & 0xFF);
    ptr8[1] = (uint16_t)((v >>  8) & 0xFF);
    ptr8[2] = (uint16_t)((v >> 16) & 0xFF);
    ptr8[3] = (uint16_t)((v >> 24) & 0xFF);
    ptr8[4] = (uint16_t)((v >> 32) & 0xFF);
    ptr8[5] = (uint16_t)((v >> 40) & 0xFF);
    ptr8[6] = (uint16_t)((v >> 48) & 0xFF);
    ptr8[7] = (uint16_t)((v >> 56) & 0xFF);
}

static inline void write16be(uint16_t *ptr, uint16_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 1) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 16);
#endif
    ptr8[0] = (uint16_t)((v >> 8) & 0xFF);
    ptr8[1] = (uint16_t)((v >> 0) & 0xFF);
}

static inline void write32be(uint32_t *ptr, uint32_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 3) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 32);
#endif
    ptr8[0] = (uint16_t)((v >> 24) & 0xFF);
    ptr8[1] = (uint16_t)((v >> 16) & 0xFF);
    ptr8[2] = (uint16_t)((v >>  8) & 0xFF);
    ptr8[3] = (uint16_t)((v >>  0) & 0xFF);
}

static inline void write64be(uint64_t *ptr, uint64_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    assert((((uintptr_t)ptr8) & 7) == 0);
#ifdef GCC_COMPILER
    ptr8 = __builtin_assume_aligned(ptr8, 64);
#endif
    ptr8[0] = (uint16_t)((v >> 56) & 0xFF);
    ptr8[1] = (uint16_t)((v >> 48) & 0xFF);
    ptr8[2] = (uint16_t)((v >> 40) & 0xFF);
    ptr8[3] = (uint16_t)((v >> 32) & 0xFF);
    ptr8[4] = (uint16_t)((v >> 24) & 0xFF);
    ptr8[5] = (uint16_t)((v >> 16) & 0xFF);
    ptr8[6] = (uint16_t)((v >>  8) & 0xFF);
    ptr8[7] = (uint16_t)((v >>  0) & 0xFF);
}

static inline void write16be_unaligned(void *ptr, uint16_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    ptr8[0] = (uint16_t)((v >> 8) & 0xFF);
    ptr8[1] = (uint16_t)((v >> 0) & 0xFF);
}

static inline void write32be_unaligned(void *ptr, uint32_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    ptr8[0] = (uint16_t)((v >> 24) & 0xFF);
    ptr8[1] = (uint16_t)((v >> 16) & 0xFF);
    ptr8[2] = (uint16_t)((v >>  8) & 0xFF);
    ptr8[3] = (uint16_t)((v >>  0) & 0xFF);
}

static inline void write64be_unaligned(void *ptr, uint64_t v)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    ptr8[0] = (uint16_t)((v >> 56) & 0xFF);
    ptr8[1] = (uint16_t)((v >> 48) & 0xFF);
    ptr8[2] = (uint16_t)((v >> 40) & 0xFF);
    ptr8[3] = (uint16_t)((v >> 32) & 0xFF);
    ptr8[4] = (uint16_t)((v >> 24) & 0xFF);
    ptr8[5] = (uint16_t)((v >> 16) & 0xFF);
    ptr8[6] = (uint16_t)((v >>  8) & 0xFF);
    ptr8[7] = (uint16_t)((v >>  0) & 0xFF);
}

static inline void memfrobn(void *ptr, size_t n, void *key, size_t keylen)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    uint8_t *key8 = (uint8_t *)key;
    for (size_t i = 0; i < n; i++) {
        ptr8[i] = ptr8[i] ^ key8[i % keylen];
    }
}

#endif /* HGL_MEM_H */

