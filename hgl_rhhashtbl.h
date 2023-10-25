
/*--- Helper macros ---------------------------------------------------------------------*/

#define _CONCAT_NX3(a, b, c) a ## b ## c
#define _CONCAT3(a, b, c) _CONCAT_NX3(a, b, c)

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

/*--- robin hood hash table-specific macros ---------------------------------------------*/

/* CONFIGURABLE: HGL_RHHASHTBL_KEY_TYPE & HGL_RHHASHTBL_VALUE_TYPE & HGL_RHHASHTBL_ID */
#if !defined(HGL_RHHASHTBL_KEY_TYPE) && \
    !defined(HGL_RHHASHTBL_VALUE_TYPE) && \
    !defined(HGL_RHHASHTBL_ID)
#define HGL_RHHASHTBL_KEY_TYPE int
#define HGL_RHHASHTBL_VALUE_TYPE float
#define HGL_RHHASHTBL_ID itof
#endif /* HGL_RHHASHTBL_TYPE */

/* CONFIGURABLE: HGL_RHHASHTBL_ALLOCATOR, HGL_RHHASHTBL_REALLOCATOR, HGL_RHHASHTBL_FREE */
#if !defined(HGL_RHHASHTBL_ALLOCATOR) && \
    !defined(HGL_RHHASHTBL_REALLOCATOR) && \
    !defined(HGL_RHHASHTBL_FREE)
#define HGL_RHHASHTBL_ALLOCATOR (malloc)
#define HGL_RHHASHTBL_REALLOCATOR (realloc)
#define HGL_RHHASHTBL_FREE (free)
#endif

/* CONFIGURABLE: HGL_RHHASHTBL_LOAD_FACTOR_THRESHOLD */
#ifndef HGL_RHHASHTBL_LOAD_FACTOR_THRESHOLD
#define HGL_RHHASHTBL_LOAD_FACTOR_THRESHOLD 0.5
//#define HGL_RHHASHTBL_LOAD_FACTOR_THRESHOLD 0.875
#endif

/* 
 * CONFIGURABLE: HGL_RHHASHTBL_HASH_FUNC, HGL_RHHASHTBL_HASH_FUNC_SEED
 *
 * Should be of type `void (*f)(const void *, int, uint32_t, void *)`.
 */
#ifndef HGL_RHHASHTBL_HASH_FUNC
//#define HGL_RHHASHTBL_HASH_FUNC (MurmurHash3_x86_32) 
#define HGL_RHHASHTBL_HASH_FUNC (djb2) 
#endif
#ifndef HGL_RHHASHTBL_HASH_FUNC_SEED
#define HGL_RHHASHTBL_HASH_FUNC_SEED 0x1337
#endif


/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

/*--- Public type definitions -----------------------------------------------------------*/

typedef void (*hgl_hash_func_t)(const void *, int, uint32_t, void *);

#define HGL_RHHASHTBL_BUCKET_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _bucket_t)
typedef struct
{
    int8_t psl;
    HGL_RHHASHTBL_KEY_TYPE key;
    HGL_RHHASHTBL_VALUE_TYPE value;
} HGL_RHHASHTBL_BUCKET_NAME;

#define HGL_RHHASHTBL_STRUCT_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _rhhashtbl_t)
typedef struct
{
    HGL_RHHASHTBL_BUCKET_NAME *tbl;
    uint32_t n_occupied_buckets;
    uint32_t n_buckets;
} HGL_RHHASHTBL_STRUCT_NAME;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Helper functions ------------------------------------------------------------------*/

#ifndef HGL_RRHASHTBL_GENERIC_HELPER_FUNCTIONS
#define HGL_RRHASHTBL_GENERIC_HELPER_FUNCTIONS

/**
 * A shit hash function.
 */
void shitty_hash(const void *key, int len, uint32_t seed, uint32_t *out)
{
    (void) key;
    (void) len;
    (void) seed;
    *out = 123456;
}

/**
 * A simple default hash function.
 */
void djb2(const void *key, int len, uint32_t seed, uint32_t *out)
{
    const uint8_t *buf = (const uint8_t *)key;
    uint32_t hash = seed;

    for (int i = 0; i < len; ++i) {
        hash = ((hash << 5) + hash) + (uint32_t)buf[i]; /* hash * 33 + c */
    }

    *out = hash;
}


/**
 * A good default hash function.
 *
 * Slightly modified from: https://github.com/PeterScott/murmur3/blob/master/murmur3.c
 */
void MurmurHash3_x86_32(const void *key, int len, uint32_t seed, uint32_t *out)
{
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 4;
    int i;

    uint32_t h1 = seed;

    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;

    //----------
    // body

    const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

    for(i = -nblocks; i; i++)
    {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        //k1 = rotl32(k1,15);
        k1 = (k1 << 15) | (k1 >> (32 - 15));
        k1 *= c2;

        h1 ^= k1;
        //h1 = rotl32(h1,13); 
        h1 = (h1 << 13) | (h1 >> (32 - 13));
        h1 = h1*5+0xe6546b64;
    }

    //----------
    // tail

    const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

    uint32_t k1 = 0;

    switch(len & 3)
    {
        case 3: k1 ^= tail[2] << 16; // fall through
        case 2: k1 ^= tail[1] << 8;  // fall through
        case 1: k1 ^= tail[0];       // fall through
                k1 *= c1;
                //k1 = rotl32(k1,15);
                k1 = (k1 << 15) | (k1 >> (32 - 15));
                k1 *= c2;
                h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len;

    //h1 = fmix32(h1);
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    *out = h1;
}

static inline uint32_t next_power_of_2(uint32_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

#endif /* HGL_RRHASHTBL_GENERIC_HELPER_FUNCTIONS */

#define HGL_RHHASHTBL_FUNC_GROW_AND_REHASH_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _rhhashtbl_grow_and_rehash_)
#include <stdio.h> // DEBUG
static inline void HGL_RHHASHTBL_FUNC_INIT_NAME(HGL_RHHASHTBL_STRUCT_NAME *hash_table)
{
    (void) hash_table;
    printf("grow and rehash!\n"); // DEBUG
}

/*--- Channel functions -----------------------------------------------------------------*/

#define HGL_RHHASHTBL_FUNC_INIT_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _rhhashtbl_init)
static inline void HGL_RHHASHTBL_FUNC_INIT_NAME(HGL_RHHASHTBL_STRUCT_NAME *hash_table, 
                                                uint32_t min_n_buckets)
{
    hash_table->n_occupied_buckets = 0;
    hash_table->n_buckets = next_power_of_2(min_n_buckets); 
    hash_table->tbl = HGL_RHHASHTBL_ALLOCATOR(hash_table->n_buckets * 
                                              sizeof(HGL_RHHASHTBL_BUCKET_NAME)); 
    /* init all psls to -1 (free slot) */
    if (hash_table->tbl != NULL) {
        for (uint32_t i = 0; i < hash_table->n_buckets; i++) {
            hash_table->tbl[i].psl = -1;
        }    
    }
}

#define HGL_RHHASHTBL_FUNC_FREE_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _rhhashtbl_free)
static inline void HGL_RHHASHTBL_FUNC_FREE_NAME(HGL_RHHASHTBL_STRUCT_NAME *hash_table)
{
    HGL_RHHASHTBL_FREE(hash_table->tbl);
}

#define HGL_RHHASHTBL_FUNC_INSERT_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _rhhashtbl_insert)
static inline void HGL_RHHASHTBL_FUNC_INSERT_NAME(HGL_RHHASHTBL_STRUCT_NAME *hash_table,
                                                  HGL_RHHASHTBL_KEY_TYPE *key,
                                                  HGL_RHHASHTBL_VALUE_TYPE *value)
{
    // TODO Check if grow and rehash necessary

    uint32_t hash;
    uint32_t index;
    HGL_RHHASHTBL_HASH_FUNC(key, sizeof(HGL_RHHASHTBL_KEY_TYPE), HGL_RHHASHTBL_HASH_FUNC_SEED, &hash);
    index = hash & (hash_table->n_buckets - 1);

    printf("insert: key = %d, hash = %u, index = %u \n", *key, hash, index);

    HGL_RHHASHTBL_BUCKET_NAME temp_bucket;
    HGL_RHHASHTBL_BUCKET_NAME bucket_to_insert = {
        .psl = 0,
        .key = *key,
        .value = *value
    };
    
    // TODO make max probe sequence length smaller and rehash if we go over the limit. 
    // Do benchmarks to find a good limit.
    for (uint32_t i = 0; i < hash_table->n_buckets; i++) {
    
        /* get the bucket at `index` in the table */
        HGL_RHHASHTBL_BUCKET_NAME *bucket_in_tbl = &hash_table->tbl[index];

        /* bucket is free: insert */
        if (bucket_in_tbl->psl == -1) {
            *bucket_in_tbl = bucket_to_insert;
            hash_table->n_occupied_buckets++;
            return;
        }

        /* key collision: replace */
        if (bucket_in_tbl->key == bucket_to_insert.key) {
            *bucket_in_tbl = bucket_to_insert;
            return;
        }

        /* take from the rich, i.e. swap buckets */
        if (bucket_in_tbl->psl < bucket_to_insert.psl) {
            temp_bucket = bucket_to_insert;
            bucket_to_insert = *bucket_in_tbl;
            *bucket_in_tbl = temp_bucket;
        }
        
        /* go to next slot and increment PSL */
        index++;
        index &= (hash_table->n_buckets - 1);
        bucket_to_insert.psl++;
    }

    printf("OOPSIE DAISIESS!! Hash table overflow! >.<");
    abort();
}

#define HGL_RHHASHTBL_FUNC_INSERT_VALUE_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _rhhashtbl_insert_by_value)
static inline void HGL_RHHASHTBL_FUNC_INSERT_VALUE_NAME(HGL_RHHASHTBL_STRUCT_NAME *hash_table,
                                                        HGL_RHHASHTBL_KEY_TYPE key,
                                                        HGL_RHHASHTBL_VALUE_TYPE value)
{
    HGL_RHHASHTBL_FUNC_INSERT_NAME(hash_table, &key, &value);
}

#define HGL_RHHASHTBL_FUNC_LOOKUP_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _rhhashtbl_lookup)
static inline HGL_RHHASHTBL_VALUE_TYPE *HGL_RHHASHTBL_FUNC_LOOKUP_NAME(HGL_RHHASHTBL_STRUCT_NAME *hash_table,
                                                                        HGL_RHHASHTBL_KEY_TYPE *key)
{
    // TODO Check if grow and rehash necessary

    uint32_t hash;
    uint32_t index;
    HGL_RHHASHTBL_HASH_FUNC(key, sizeof(HGL_RHHASHTBL_KEY_TYPE), HGL_RHHASHTBL_HASH_FUNC_SEED, &hash);
    index = hash & (hash_table->n_buckets - 1);
    
    printf("lookup: key = %d, hash = %u, index = %u \n", *key, hash, index);

    // TODO make max probe sequence length smaller and rehash if we go over the limit. 
    // Do benchmarks to find a good limit.
    for (int8_t i = 0; i < 127; i++) {
    
        /* get the bucket at `index` in the table */
        HGL_RHHASHTBL_BUCKET_NAME *bucket_in_tbl = &hash_table->tbl[index];

        /* bucket is free or bucket has PSL higher than what the sought after key 
         * would have in that slot: no key exists. return NULL */
        //if (bucket_in_tbl->psl == -1 || bucket_in_tbl->psl > i) {
        if (bucket_in_tbl->psl == -1 || bucket_in_tbl->psl < i) {
            return NULL;
        }

        /* key found: return it */
        if (*key == bucket_in_tbl->key) {
            return &bucket_in_tbl->value;
        }

        /* go to next slot */
        index++;
        index &= (hash_table->n_buckets - 1);
    }

    printf("Oppsiess, key not found");
    return NULL;
}

#define HGL_RHHASHTBL_FUNC_LOOKUP_VALUE_NAME _CONCAT3(hgl_, HGL_RHHASHTBL_ID, _rhhashtbl_lookup_by_value)
static inline HGL_RHHASHTBL_VALUE_TYPE *HGL_RHHASHTBL_FUNC_LOOKUP_VALUE_NAME(HGL_RHHASHTBL_STRUCT_NAME *hash_table,
                                                                             HGL_RHHASHTBL_KEY_TYPE key)
{
    return HGL_RHHASHTBL_FUNC_LOOKUP_NAME(hash_table, &key);
}
