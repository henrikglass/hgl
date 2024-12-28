
#include "hgl_test.h"

#define HGL_HTABLE_IMPLEMENTATION
#include "hgl_htable.h"

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

static HglHashTable htbl;

GLOBAL_SETUP 
{
    htbl = hgl_htable_make(djb2, keysize, keycmp, 8, true);
    hgl_htable_insert(&htbl, "John", "Carpenter");
    hgl_htable_insert(&htbl, "Frank", "Plumber");
    hgl_htable_insert(&htbl, "Lisa", "Electrician");
    hgl_htable_insert(&htbl, "Homer", "Nuclear power plant operator");
    hgl_htable_insert(&htbl, "Marge", "Unemployed");
}

GLOBAL_TEARDOWN 
{
    hgl_htable_destroy(&htbl); 
}

TEST(test_load_factor) 
{
    ASSERT(htbl.n_occupied_buckets == 5);
    ASSERT(htbl.capacity == 16);
}

TEST(test_load_factor2) 
{
    hgl_htable_insert(&htbl, "A", "a");
    hgl_htable_insert(&htbl, "B", "b");
    hgl_htable_insert(&htbl, "C", "c");
    ASSERT(htbl.capacity == 16);
    hgl_htable_insert(&htbl, "D", "d");
    ASSERT(htbl.capacity == 32);
}

TEST(test_lookup, .expect_output = "Unemployed\n"
                                   "Nuclear power plant operator\n") 
{
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Marge"));
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Homer"));
}

TEST(test_insert, .expect_output = "Unemployed\n"
                                   "Nuclear power plant operator\n"
                                   "Elementary school principal\n") 
{
    hgl_htable_insert(&htbl, "Seymour", "Elementary school principal");
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Marge"));
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Homer"));
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Seymour"));
}

TEST(test_overwrite, .expect_output = "Nuclear power plant operator\n"
                                      "Car Designer\n") 
{
    ASSERT(htbl.n_occupied_buckets == 5);
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Homer"));
    hgl_htable_insert(&htbl, "Homer", "Car Designer");
    ASSERT(htbl.n_occupied_buckets == 5);
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Homer"));
}

TEST(test_remove) 
{
    ASSERT(htbl.n_occupied_buckets == 5);
    hgl_htable_remove(&htbl, "Homer");
    ASSERT(NULL == hgl_htable_lookup(&htbl, "Homer"));
    ASSERT(htbl.n_occupied_buckets == 4);
}

TEST(test_join, .expect_output = "Unemployed\n"
                                 "Nuclear power plant operator\n"
                                 "Power plant worker\n"
                                 "Bartender\n") 
{
    HglHashTable htbl2 = hgl_htable_make(djb2, keysize, keycmp, 4, true);
    hgl_htable_insert(&htbl2, "Seymour", "Elementary school principal");
    hgl_htable_insert(&htbl2, "Lenny", "Power plant worker");
    hgl_htable_insert(&htbl2, "Carl", "Power plant worker");
    hgl_htable_insert(&htbl2, "Moe", "Bartender");
    hgl_htable_join(&htbl, &htbl2);

    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Marge"));
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Homer"));
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Lenny"));
    printf("%s\n", (char *) hgl_htable_lookup(&htbl, "Moe"));
}
