
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
 * hgl_hset.h implements a simple to use robin hood hash set. 
 *
 *
 * USAGE:
 *
 * Include hgl_hset.h file like this:
 *
 *     #define HGL_HSET_IMPLEMENTATION
 *     #include "hgl_hset.h"
 *
 * You can redefine the load factor threshold, hash seed, and the default allocator:
 *
 *     HGL_HSET_LOAD_FACTOR_THRESHOLD  (default: 0.5f)
 *     HGL_HSET_HASH_FUNC_SEED         (default: 0x1337)
 *     HGL_HSET_ALLOC                  (default: malloc)
 *     HGL_HSET_FREE                   (default: free)
 *
 * Example:
 *
 *     
 *     static inline int keycmp(const void *key1, const void *key2) {
 *         return strcmp((const char *)key1, (const char *)key2);
 *     }
 *     
 *     static inline size_t keysize(const void *key) {
 *         return strlen((const char *) key);
 *     }
 *     
 *     static inline void djb2(const void *key, size_t len, uint32_t seed, uint32_t *out) {
 *         const uint8_t *buf = (const uint8_t *)key;
 *         uint32_t hash = seed;
 *         for (size_t i = 0; i < len; ++i) {
 *             hash = ((hash << 5) + hash) + (uint32_t)buf[i];
 *         }
 *         *out = hash;
 *     }
 *
 *                    /.../
 *
 *     HglHashSet friends = hgl_hset_make(djb2,    // hash function ptr
 *                                        keysize, // keysize function ptr
 *                                        keycmp,  // keycmp function ptr
 *                                        4,       // initial size
 *                                        true);   // growable? (may resize above user-specified threshold)
 *     if (friends.buckets == NULL) {
 *         return 1; // error 
 *     }
 *
 *     hgl_hset_insert(&friends, "John");
 *     hgl_hset_insert(&friends, "Frank");
 *     hgl_hset_insert(&friends, "Phil");
 *     hgl_hset_insert(&friends, "Scott");
 *     hgl_hset_insert(&friends, "Dwight");
 *     hgl_hset_insert(&friends, "Mudasir");
 *
 *     // Friendship ended with MUDASIR. Now SALMAN is my best friend.
 *     hgl_hset_remove(&friends, "Mudasir");
 *     hgl_hset_insert(&friends, "Salman");
 *
 *     if (hgl_hset_contains(&friends, "Scott")) {
 *         printf("I'm friends with Scott!\n");
 *     }
 *
 *     hgl_hset_destroy(&friends);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#define HGL_HSET_IMPLEMENTATION
#include "hgl_hset.h"

#include <stdio.h>
#include <string.h>

static inline int my_keycmp_func(const void *key1, const void *key2)
{
    return strcmp((const char *)key1, (const char *)key2);
}

static inline size_t my_keysize_func(const void *key)
{
    return strlen((const char *) key);
}

static inline void djb2(const void *key, size_t len, uint32_t seed, uint32_t *out)
{
    const uint8_t *buf = (const uint8_t *)key;
    uint32_t hash = seed;

    for (size_t i = 0; i < len; ++i) {
        hash = ((hash << 5) + hash) + (uint32_t)buf[i]; /* hash * 33 + c */
    }

    *out = hash;
}

#define PRINT_SET                                                                                \
    do {                                                                                         \
        printf("-------------------- SET ----------------------\n");                             \
        for (uint32_t i = 0; i < set.capacity; i++) {                                            \
            if (set.buckets[i].psl == -1) {                                                      \
                printf("[%u] --- \n", i);                                                        \
            } else {                                                                             \
                printf("[%u] \"%s\" @%d\n", i, (char *) set.buckets[i].key, set.buckets[i].psl); \
            }                                                                                    \
        }                                                                                        \
        printf("-----------------------------------------------\n");                             \
    } while (0)

int main(void)
{
    printf("Hello World!\n");
    
    HglHashSet set = hgl_hset_make(djb2, my_keysize_func, my_keycmp_func, 8, false);

    hgl_hset_insert(&set, "Bob");
    hgl_hset_insert(&set, "Alice");
    hgl_hset_insert(&set, "Cooper");
    hgl_hset_insert(&set, "Dylan");
    hgl_hset_insert(&set, "Eric");
    hgl_hset_insert(&set, "Fred");
    hgl_hset_insert(&set, "Cooper");
    hgl_hset_insert(&set, "Cooper");
    hgl_hset_insert(&set, "Dylan");
    hgl_hset_insert(&set, "Dylan");
    assert(hgl_hset_contains(&set, "Alice"));
    assert(hgl_hset_contains(&set, "Dylan"));
    assert(hgl_hset_contains(&set, "Eric"));

    hgl_hset_insert(&set, "Cartman");
    hgl_hset_insert(&set, "Kyle");

#if 0
    /* this should error */
    hgl_hset_insert(&set, "Kenny");
    hgl_hset_insert(&set, "Stan");
#endif

    PRINT_SET;

    hgl_hset_remove(&set, "Cooper");
    hgl_hset_remove(&set, "Alice");

    assert(!hgl_hset_contains(&set, "Alice"));
    assert(!hgl_hset_contains(&set, "Cooper"));
    assert(hgl_hset_contains(&set, "Dylan"));
    assert(hgl_hset_contains(&set, "Eric"));

    PRINT_SET;

    hgl_hset_destroy(&set);
    return 0;
}
