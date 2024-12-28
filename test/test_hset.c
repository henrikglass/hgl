
#include "hgl_test.h"

#define HGL_HSET_IMPLEMENTATION
#include "hgl_hset.h"

static inline int keycmp(const void *key1, const void *key2)
{
    return strcmp((const char *)key1, (const char *)key2);
}

static inline size_t keysize(const void *key)
{
    return strlen((const char *) key);
}

static inline void djb2(const void *key, size_t len, uint32_t seed, uint32_t *out)
{
    const uint8_t *buf = (const uint8_t *)key;
    uint32_t hash = seed;
    for (size_t i = 0; i < len; ++i) {
        hash = ((hash << 5) + hash) + (uint32_t)buf[i];
    }
    *out = hash;
}

static HglHashSet hset;

GLOBAL_SETUP 
{
    hset = hgl_hset_make(djb2, keysize, keycmp, 8, true);
    hgl_hset_insert(&hset, "John");
    hgl_hset_insert(&hset, "Frank");
    hgl_hset_insert(&hset, "Lisa");
    hgl_hset_insert(&hset, "Homer");
    hgl_hset_insert(&hset, "Marge");
}

GLOBAL_TEARDOWN 
{
    hgl_hset_destroy(&hset); 
}

TEST(test_load_factor) 
{
    ASSERT(hset.n_occupied_buckets == 5);
    ASSERT(hset.capacity == 16);
}

TEST(test_load_factor2) 
{
    hgl_hset_insert(&hset, "A");
    hgl_hset_insert(&hset, "B");
    hgl_hset_insert(&hset, "C");
    ASSERT(hset.capacity == 16);
    hgl_hset_insert(&hset, "D");
    ASSERT(hset.capacity == 32);
}

TEST(test_lookup) 
{
    ASSERT(hgl_hset_contains(&hset, "Marge"));
    ASSERT(hgl_hset_contains(&hset, "Homer"));
}

TEST(test_insert) 
{
    hgl_hset_insert(&hset, "Seymour");
    ASSERT(hgl_hset_contains(&hset, "Marge"));
    ASSERT(hgl_hset_contains(&hset, "Homer"));
    ASSERT(hgl_hset_contains(&hset, "Seymour"));
}

TEST(test_overwrite) 
{
    ASSERT(hset.n_occupied_buckets == 5);
    hgl_hset_insert(&hset, "Homer");
    ASSERT(hset.n_occupied_buckets == 5);
}

TEST(test_remove) 
{
    ASSERT(hset.n_occupied_buckets == 5);
    hgl_hset_remove(&hset, "Homer");
    ASSERT(!hgl_hset_contains(&hset, "Homer"));
    ASSERT(hset.n_occupied_buckets == 4);
}

TEST(test_join) 
{
    HglHashSet hset2 = hgl_hset_make(djb2, keysize, keycmp, 4, true);
    hgl_hset_insert(&hset2, "Seymour");
    hgl_hset_insert(&hset2, "Lenny");
    hgl_hset_insert(&hset2, "Carl");
    hgl_hset_insert(&hset2, "Moe");
    hgl_hset_join(&hset, &hset2);
    ASSERT(hgl_hset_contains(&hset, "Marge"));
    ASSERT(hgl_hset_contains(&hset, "Homer"));
    ASSERT(hgl_hset_contains(&hset, "Seymour"));
    ASSERT(hgl_hset_contains(&hset, "Carl"));
    ASSERT(hgl_hset_contains(&hset, "Moe"));
    ASSERT(hgl_hset_contains(&hset, "Lenny"));
}
