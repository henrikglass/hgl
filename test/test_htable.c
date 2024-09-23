
#define HGL_HTABLE_IMPLEMENTATION
#include "hgl_htable.h"

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

static inline void my_shitty_hash_func(const void *key, size_t len, uint32_t seed, uint32_t *out)
{
    (void) key;
    (void) len;
    (void) seed;
    *out = 123456;
}

static inline void my_id_hash_func(const void *key, size_t len, uint32_t seed, uint32_t *out)
{
    (void) key;
    (void) len;
    (void) seed;
    *out = *(const uint32_t *) key;
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

#define PRINT_TABLE                                                                                \
    do {                                                                                           \
        printf("------------------- TABLE ---------------------\n");                               \
        for (uint32_t i = 0; i < htbl.capacity; i++) {                                             \
            if (htbl.buckets[i].psl == -1) {                                                       \
                printf("[%u] --- \n", i);                                                          \
            } else {                                                                               \
                printf("[%u] \"%s\" @%d\n", i, (char *) htbl.buckets[i].key, htbl.buckets[i].psl); \
            }                                                                                      \
        }                                                                                          \
        printf("-----------------------------------------------\n");                               \
    } while (0)

int main(void)
{
    printf("Hello World!\n");


    HglHashTable htbl = hgl_htable_make(djb2, my_keysize_func, my_keycmp_func, 4, true);
    if (htbl.buckets == NULL) {
        return -1;
    }

    hgl_htable_insert(&htbl, "alice", "client");
    hgl_htable_insert(&htbl, "bob", "server");
    hgl_htable_insert(&htbl, "cooper", "astronaut");
    hgl_htable_insert(&htbl, "dylan", "unemployed");
    hgl_htable_insert(&htbl, "a", "testa");
    hgl_htable_insert(&htbl, "b", "testb");
    hgl_htable_insert(&htbl, "c", "testc");
    hgl_htable_insert(&htbl, "d", "testd");
    hgl_htable_insert(&htbl, "e", "teste");

    PRINT_TABLE;

    hgl_htable_remove(&htbl, "d");
    hgl_htable_remove(&htbl, "alice");
    hgl_htable_remove(&htbl, "dylan");

    PRINT_TABLE;

    void *value = hgl_htable_lookup(&htbl, "alice");
    if (value != NULL) {
        printf("found: %s\n", (char *)value);
    }
    value = hgl_htable_lookup(&htbl, "bob");
    if (value != NULL) {
        printf("found: %s\n", (char *)value);
    }
    value = hgl_htable_lookup(&htbl, "cooper");
    if (value != NULL) {
        printf("found: %s\n", (char *)value);
    }
    value = hgl_htable_lookup(&htbl, "dylan");
    if (value != NULL) {
        printf("found: %s\n", (char *)value);
    }
    value = hgl_htable_lookup(&htbl, "Eric");
    if (value != NULL) {
        printf("found: %s\n", (char *)value);
    }

    hgl_htable_destroy(&htbl);

    return 0;
}
