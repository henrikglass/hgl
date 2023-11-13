
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
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
 * hgl_hash_table.h implements a simple (ok maybe not...) to use robin hood hash table. 
 *
 *
 * USAGE:
 *
 * Include hgl_hash_table.h file like this:
 *
 *     #define HGL_HASH_TABLE_KEY_TYPE int
 *     #define HGL_HASH_TABLE_VALUE_TYPE int
 *     #define HGL_HASH_TABLE_ID itoi
 *     #include "hgl_hash_table.h"
 *
 * This will create an implementation of hgl_hash_table capable of holding <int,int> key-value 
 * pairs. "itoi" is used as an infix in the identifiers exposed by the library. Below is a 
 * complete list of the generated API:
 *
 *     void hgl_itoi_hash_table_init(hgl_itoi_hash_table_t *hash_table, size_t min_n_buckets);
 *     void hgl_itoi_hash_table_free(hgl_itoi_hash_table_t *hash_table);
 *     void hgl_itoi_hash_table_insert(hgl_itoi_hash_table_t *hash_table, int *key, int *value);    // pass k,v as pointers
 *     void hgl_itoi_hash_table_insert_cpkv(hgl_itoi_hash_table_t *hash_table, int key, int value); // pass k,v as values (copy)
 *     void hgl_itoi_hash_table_insert_cpv(hgl_itoi_hash_table_t *hash_table, int *key, int value); // pass k as pointer, v as value (copy)
 *     int *hgl_itoi_hash_table_lookup(hgl_itoi_hash_table_t *hash_table, int *key);                // pass k as pointer
 *     int *hgl_itoi_hash_table_lookup_cp(hgl_itoi_hash_table_t *hash_table, int key);              // pass k as value (copy)
 *     void hgl_itoi_hash_table_remove(hgl_itoi_hash_table_t *hash_table, int *key);                // pass k as pointer
 *     void hgl_itoi_hash_table_remove_cp(hgl_itoi_hash_table_t *hash_table, int key);              // pass k as value (copy)
 *
 * To create a hash table that maps between complex types, it will be necessary to redefine one or more 
 * of the following default function name macros with your own-supplied functions:
 * 
 *     HGL_HASH_TABLE_KEYSIZE_FUNC     (default: default_keysize - works with fundamental numeric types and struct. 
 *                                      Also supplied with the library: strlen_keysize)
 *     HGL_HASH_TABLE_KEYCMP_FUNC      (default: default_keycmp - works with fundamental numeric types. Also 
 *                                      supplied with the library: strcmp_keycmp)
 *
 * You may also supply your own hash function by redefining the macro
 *
 *     HGL_HASH_TABLE_HASH_FUNC        (default: MurmurHash3_x86_32, but a few others are supplied in the library:
 *                                      djb2, identity_hash32, and shitty_hash)
 * 
 * These three user-supplied functions have the prototypes:
 *
 *     typedef void (*hgl_hash_func_t)(const void *key, int len, uint32_t seed, void *out);
 *     typedef void (*hgl_keysize_func_t)(const void *key);
 *     typedef int (*hgl_keycmp_func_t)(const void *key1, const void *key2);
 *
 * Naturally, you can also redefine the load factor threshold, hash seed, and the default allocator 
 * and free- functions:
 *
 *     HGL_HASH_TABLE_LOAD_FACTOR_THRESHOLD  (default: 0.5)
 *     HGL_HASH_TABLE_HASH_FUNC_SEED         (default: 1337)
 *     HGL_HASH_TABLE_ALLOCATOR              (default: malloc)
 *     HGL_HASH_TABLE_FREE                   (default: free)
 *
 * To include multiple implementations of hgl_hash_table, simply redefine the necessary macro defines
 * and include hgl_hash_table.h again:
 *     
 *     // First include
 *     #define HGL_HASH_TABLE_KEY_TYPE int
 *     #define HGL_HASH_TABLE_VALUE_TYPE int
 *     #define HGL_HASH_TABLE_ID itoi
 *     #include "hgl_hash_table.h"
 *
 *     // undefine previous defines
 *     #undef HGL_HASH_TABLE_KEY_TYPE
 *     #undef HGL_HASH_TABLE_VALUE_TYPE
 *     #undef HGL_HASH_TABLE_ID
 *     #undef HGL_HASH_TABLE_KEYSIZE_FUNC
 *     #undef HGL_HASH_TABLE_KEYCMP_FUNC
 *
 *     // define necessary macros again
 *     #define HGL_HASH_TABLE_KEY_TYPE const char *
 *     #define HGL_HASH_TABLE_VALUE_TYPE int
 *     #define HGL_HASH_TABLE_ID strtoi
 *     #define HGL_HASH_TABLE_KEYSIZE_FUNC (strlen_keysize)
 *     #define HGL_HASH_TABLE_KEYCMP_FUNC (strcmp_keycmp)
 *     #include "hgl_hash_table.h"
 *
 * AUTHOR: Henrik A. Glass
 *
 */

/*--- Helper macros ---------------------------------------------------------------------*/

#define _CONCAT_NX3(a, b, c) a ## b ## c
#define _CONCAT3(a, b, c) _CONCAT_NX3(a, b, c)

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

/*--- robin hood hash table-specific macros ---------------------------------------------*/

/* CONFIGURABLE: HGL_HASH_TABLE_KEY_TYPE & HGL_HASH_TABLE_VALUE_TYPE & HGL_HASH_TABLE_ID */
#if !defined(HGL_HASH_TABLE_KEY_TYPE) && \
    !defined(HGL_HASH_TABLE_VALUE_TYPE) && \
    !defined(HGL_HASH_TABLE_ID)
#define HGL_HASH_TABLE_KEY_TYPE int
#define HGL_HASH_TABLE_VALUE_TYPE float
#define HGL_HASH_TABLE_ID itof
#endif /* HGL_HASH_TABLE_TYPE */

/* CONFIGURABLE: HGL_HASH_TABLE_ALLOCATOR, HGL_HASH_TABLE_FREE */
#if !defined(HGL_HASH_TABLE_ALLOCATOR) && \
    !defined(HGL_HASH_TABLE_FREE)
#define HGL_HASH_TABLE_ALLOCATOR (malloc)
#define HGL_HASH_TABLE_FREE (free)
#endif

/* CONFIGURABLE: HGL_HASH_TABLE_LOAD_FACTOR_THRESHOLD */
#ifndef HGL_HASH_TABLE_LOAD_FACTOR_THRESHOLD
#define HGL_HASH_TABLE_LOAD_FACTOR_THRESHOLD 0.5
//#define HGL_HASH_TABLE_LOAD_FACTOR_THRESHOLD 0.875
#endif

/* 
 * CONFIGURABLE: HGL_HASH_TABLE_HASH_FUNC, HGL_HASH_TABLE_HASH_FUNC_SEED
 *
 * Should be of type `void (*f)(const void *, int, uint32_t, void *)`.
 */
#ifndef HGL_HASH_TABLE_HASH_FUNC
#define HGL_HASH_TABLE_HASH_FUNC (MurmurHash3_x86_32) 
//#define HGL_HASH_TABLE_HASH_FUNC (djb2) 
//#define HGL_HASH_TABLE_HASH_FUNC (identity_hash32) 
//#define HGL_HASH_TABLE_HASH_FUNC (shitty_hash) 
#endif
#ifndef HGL_HASH_TABLE_HASH_FUNC_SEED
#define HGL_HASH_TABLE_HASH_FUNC_SEED 0x1337
#endif

#ifndef HGL_HASH_TABLE_KEYSIZE_FUNC
#define HGL_HASH_TABLE_KEYSIZE_FUNC (default_keysize)
//#define HGL_HASH_TABLE_KEYSIZE_FUNC (strlen_keysize)
#endif

#ifndef HGL_HASH_TABLE_KEYCMP_FUNC
#define HGL_HASH_TABLE_KEYCMP_FUNC (default_keycmp)
//#define HGL_HASH_TABLE_KEYCMP_FUNC (strcmp_keycmp)
#endif

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h> // fprintf
#include <string.h> // strlen

/*--- Public type definitions -----------------------------------------------------------*/

typedef void (*hgl_hash_func_t)(const void *, int, uint32_t, void *);
typedef void (*hgl_keysize_func_t)(const void *);
typedef int (*hgl_keycmp_func_t)(const void *, const void *);

#define HGL_HASH_TABLE_BUCKET _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _bucket_t)
typedef struct
{
    int8_t psl;
    HGL_HASH_TABLE_KEY_TYPE key;
    HGL_HASH_TABLE_VALUE_TYPE value;
} HGL_HASH_TABLE_BUCKET;

#define HGL_HASH_TABLE_STRUCT _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_t)
typedef struct
{
    HGL_HASH_TABLE_BUCKET *tbl;
    uint32_t n_occupied_buckets;
    uint32_t n_buckets;
} HGL_HASH_TABLE_STRUCT;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Helper functions ------------------------------------------------------------------*/

#ifndef HGL_HASH_TABLE_GENERIC_HELPER_FUNCTIONS
#define HGL_HASH_TABLE_GENERIC_HELPER_FUNCTIONS

/**
 * Default key compare function. Works if key is a fundamental built-in type.
 */
static inline int default_keycmp(const void *key1, const void *key2)
{
    return (*(HGL_HASH_TABLE_KEY_TYPE *) key2) - (*(HGL_HASH_TABLE_KEY_TYPE *) key1);
}

/**
 * A key compare function that uses strcmp. Works for null-terminated strings.
 */
static inline int strcmp_keycmp(const void *key1, const void *key2)
{
    return strcmp(*(const char **)key1, *(const char **)key2);
}

/**
 * Default keysize function. Works if key is a fundamental built-in type or struct.
 */
static inline size_t default_keysize(const void *key)
{
    (void) key;
    return sizeof(HGL_HASH_TABLE_KEY_TYPE);
}

/**
 * A keysize function that uses strlen. Works for null-terminated strings.
 */
static inline size_t strlen_keysize(const void *key)
{
    return strlen((const char *) key);
}

/**
 * A shit hash function.
 */
static inline void shitty_hash(const void *key, int len, uint32_t seed, uint32_t *out)
{
    (void) key;
    (void) len;
    (void) seed;
    *out = 123456;
}

/**
 * A hash function that produces the identity for 32 bit keys.
 */
static inline void identity_hash32(const void *key, int len, uint32_t seed, uint32_t *out)
{
    (void) key;
    (void) len;
    (void) seed;
    *out = *(uint32_t *) key;
}

/**
 * A simple default hash function.
 */
static inline void djb2(const void *key, int len, uint32_t seed, uint32_t *out)
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
static inline void MurmurHash3_x86_32(const void *key, int len, uint32_t seed, uint32_t *out)
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

#endif /* HGL_HASH_TABLE_GENERIC_HELPER_FUNCTIONS */

#define HGL_HASH_TABLE_FUNC_GROW_AND_REHASH _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_grow_and_rehash_)
static inline void HGL_HASH_TABLE_FUNC_GROW_AND_REHASH(HGL_HASH_TABLE_STRUCT *hash_table)
{
    /* construct a new table of double the previous size */
    hash_table->n_buckets <<= 1;
    HGL_HASH_TABLE_BUCKET *old_table = hash_table->tbl;
    hash_table->tbl = HGL_HASH_TABLE_ALLOCATOR(hash_table->n_buckets * sizeof(HGL_HASH_TABLE_BUCKET));
    hash_table->n_occupied_buckets = 0;
    if (hash_table->tbl == NULL) {
        fprintf(stderr, "Allocation failed. Buy more RAM. <%s, %d>\n", __FILE__, __LINE__);
    }

    /* init all psls to -1 (free slot) */
    for (uint32_t i = 0; i < hash_table->n_buckets; i++) {
        hash_table->tbl[i].psl = -1;
    }    
    
    /* iterate old hash table, rehash keys, and put into new table */
    uint32_t old_table_size = hash_table->n_buckets >> 1;
    for (uint32_t i = 0; i < old_table_size; i++) {
        HGL_HASH_TABLE_BUCKET *old_bucket = &old_table[i];
        if (old_bucket->psl == -1) {
            continue;
        }

        uint32_t hash;
        uint32_t index;
        HGL_HASH_TABLE_HASH_FUNC(&old_bucket->key,
                                HGL_HASH_TABLE_KEYSIZE_FUNC(&old_bucket->key),
                                HGL_HASH_TABLE_HASH_FUNC_SEED,
                                &hash);
        index = hash & (hash_table->n_buckets - 1);

        HGL_HASH_TABLE_BUCKET temp_bucket;
        HGL_HASH_TABLE_BUCKET bucket_to_insert = *old_bucket;
        bucket_to_insert.psl = 0;
        
        // TODO DRY...  
        // TODO make max probe sequence length smaller and rehash if we go over the limit. 
        // Do benchmarks to find a good limit.
        for (uint32_t i = 0; i < hash_table->n_buckets; i++) {
        
            /* get the bucket at `index` in the table */
            HGL_HASH_TABLE_BUCKET *bucket_in_tbl = &hash_table->tbl[index];

            /* bucket is free: insert */
            if (bucket_in_tbl->psl == -1) {
                *bucket_in_tbl = bucket_to_insert;
                hash_table->n_occupied_buckets++;
                break;
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
    } 
    
    /* free old table */
    HGL_HASH_TABLE_FREE(old_table);
}

/*--- Channel functions -----------------------------------------------------------------*/

#define HGL_HASH_TABLE_FUNC_INIT _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_init)
static inline void HGL_HASH_TABLE_FUNC_INIT(HGL_HASH_TABLE_STRUCT *hash_table, 
                                           uint32_t min_n_buckets)
{
    hash_table->n_occupied_buckets = 0;
    hash_table->n_buckets = next_power_of_2(min_n_buckets); 
    hash_table->tbl = HGL_HASH_TABLE_ALLOCATOR(hash_table->n_buckets * sizeof(HGL_HASH_TABLE_BUCKET)); 
    if (hash_table->tbl == NULL) {
        fprintf(stderr, "Allocation failed. Buy more RAM. <%s, %d>\n", __FILE__, __LINE__);
    }

    /* init all psls to -1 (free slot) */
    for (uint32_t i = 0; i < hash_table->n_buckets; i++) {
        hash_table->tbl[i].psl = -1;
    }    
}

#define HGL_HASH_TABLE_FUNC_FREE _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_free)
static inline void HGL_HASH_TABLE_FUNC_FREE(HGL_HASH_TABLE_STRUCT *hash_table)
{
    HGL_HASH_TABLE_FREE(hash_table->tbl);
}

#define HGL_HASH_TABLE_FUNC_INSERT _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_insert)
static inline void HGL_HASH_TABLE_FUNC_INSERT(HGL_HASH_TABLE_STRUCT *hash_table,
                                              HGL_HASH_TABLE_KEY_TYPE *key,
                                              HGL_HASH_TABLE_VALUE_TYPE *value)
{
    /* rehash if needed */
    float load_factor = (float) hash_table->n_occupied_buckets / (float) hash_table->n_buckets;
    if (load_factor > HGL_HASH_TABLE_LOAD_FACTOR_THRESHOLD) {
        HGL_HASH_TABLE_FUNC_GROW_AND_REHASH(hash_table);
    }

    uint32_t hash;
    uint32_t index;
    HGL_HASH_TABLE_HASH_FUNC(key, 
                             HGL_HASH_TABLE_KEYSIZE_FUNC(key), 
                             HGL_HASH_TABLE_HASH_FUNC_SEED, 
                             &hash);
    index = hash & (hash_table->n_buckets - 1);

    HGL_HASH_TABLE_BUCKET temp_bucket;
    HGL_HASH_TABLE_BUCKET bucket_to_insert = {
        .psl = 0,
        .key = *key,
        .value = *value
    };
    
    // TODO make max probe sequence length smaller and rehash if we go over the limit. 
    // Do benchmarks to find a good limit.
    for (uint32_t i = 0; i < hash_table->n_buckets; i++) {
    
        /* get the bucket at `index` in the table */
        HGL_HASH_TABLE_BUCKET *bucket_in_tbl = &hash_table->tbl[index];

        /* bucket is free: insert */
        if (bucket_in_tbl->psl == -1) {
            *bucket_in_tbl = bucket_to_insert;
            hash_table->n_occupied_buckets++;
            return;
        }

        /* key collision: replace */
        if (HGL_HASH_TABLE_KEYCMP_FUNC(&bucket_in_tbl->key, &bucket_to_insert.key) == 0) {
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

    printf("OOPSIE DAISIESS!! Hash table overflow!");
    abort();
}

#define HGL_HASH_TABLE_FUNC_INSERT_KV_BY_COPY _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_insert_cpkv)
static inline void HGL_HASH_TABLE_FUNC_INSERT_KV_BY_COPY(HGL_HASH_TABLE_STRUCT *hash_table,
                                                         HGL_HASH_TABLE_KEY_TYPE key,
                                                         HGL_HASH_TABLE_VALUE_TYPE value)
{
    HGL_HASH_TABLE_FUNC_INSERT(hash_table, &key, &value);
}

#define HGL_HASH_TABLE_FUNC_INSERT_V_BY_COPY _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_insert_cpv)
static inline void HGL_HASH_TABLE_FUNC_INSERT_V_BY_COPY(HGL_HASH_TABLE_STRUCT *hash_table,
                                                        HGL_HASH_TABLE_KEY_TYPE *key,
                                                        HGL_HASH_TABLE_VALUE_TYPE value)
{
    HGL_HASH_TABLE_FUNC_INSERT(hash_table, key, &value);
}

#define HGL_HASH_TABLE_FUNC_LOOKUP _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_lookup)
static inline HGL_HASH_TABLE_VALUE_TYPE *HGL_HASH_TABLE_FUNC_LOOKUP(HGL_HASH_TABLE_STRUCT *hash_table,
                                                                    HGL_HASH_TABLE_KEY_TYPE *key)
{
    uint32_t hash;
    uint32_t index;
    HGL_HASH_TABLE_HASH_FUNC(key, HGL_HASH_TABLE_KEYSIZE_FUNC(key), HGL_HASH_TABLE_HASH_FUNC_SEED, &hash);
    index = hash & (hash_table->n_buckets - 1);
    
    //printf("lookup: key = %d, hash = %u, index = %u \n", *key, hash, index);

    // TODO make max probe sequence length smaller and rehash if we go over the limit. 
    // Do benchmarks to find a good limit.
    for (int8_t i = 0; i < 127; i++) {
    
        /* get the bucket at `index` in the table */
        HGL_HASH_TABLE_BUCKET *bucket_in_tbl = &hash_table->tbl[index];

        /* bucket is free or bucket has PSL higher than what the sought after key 
         * would have in that slot: no key exists. return NULL */
        //if (bucket_in_tbl->psl == -1 || bucket_in_tbl->psl > i) {
        if (bucket_in_tbl->psl == -1 || bucket_in_tbl->psl < i) {
            return NULL;
        }

        /* key found: return it */
        if (HGL_HASH_TABLE_KEYCMP_FUNC(key, &bucket_in_tbl->key) == 0) {
            return &bucket_in_tbl->value;
        }

        /* go to next slot */
        index++;
        index &= (hash_table->n_buckets - 1);
    }

    printf("Oppsiess, key not found");
    return NULL;
}

#define HGL_HASH_TABLE_FUNC_LOOKUP_BY_COPY _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_lookup_cp)
static inline HGL_HASH_TABLE_VALUE_TYPE *HGL_HASH_TABLE_FUNC_LOOKUP_BY_COPY(HGL_HASH_TABLE_STRUCT *hash_table,
                                                                            HGL_HASH_TABLE_KEY_TYPE key)
{
    return HGL_HASH_TABLE_FUNC_LOOKUP(hash_table, &key);
}

#define HGL_HASH_TABLE_FUNC_REMOVE _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_remove)
static inline void HGL_HASH_TABLE_FUNC_REMOVE(HGL_HASH_TABLE_STRUCT *hash_table,
                                              HGL_HASH_TABLE_KEY_TYPE *key)
{
    uint32_t hash;
    uint32_t index;
    HGL_HASH_TABLE_HASH_FUNC(key, HGL_HASH_TABLE_KEYSIZE_FUNC(key), HGL_HASH_TABLE_HASH_FUNC_SEED, &hash);
    index = hash & (hash_table->n_buckets - 1);
    
    //printf("lookup: key = %d, hash = %u, index = %u \n", *key, hash, index);

    // TODO make max probe sequence length smaller and rehash if we go over the limit. 
    // Do benchmarks to find a good limit.
    for (int8_t i = 0; i < 127; i++) {
    
        /* get the bucket at `index` in the table */
        HGL_HASH_TABLE_BUCKET *bucket_in_tbl = &hash_table->tbl[index];

        /* bucket is free or bucket has PSL higher than what the sought after key 
         * would have in that slot: no key exists. return */
        if (bucket_in_tbl->psl == -1 || bucket_in_tbl->psl < i) {
            return;
        }

        /* key found: remove it and backward shift nearby buckets if possible */
        if (HGL_HASH_TABLE_KEYCMP_FUNC(key, &bucket_in_tbl->key) == 0) {
            /* set as unoccupied */
            bucket_in_tbl->psl = -1;

            /* backward shift until empty slot or bucket with PSL = 0 */
            for (uint32_t j = 0; j < hash_table->n_buckets; j++) {
                HGL_HASH_TABLE_BUCKET *next_bucket_in_tbl = 
                    &hash_table->tbl[(index + j + 1) & (hash_table->n_buckets - 1)];

                /* next slot is empty (psl = -1) or contains bucket with optimal position (psl = 0): return */
                if (next_bucket_in_tbl->psl < 1) {
                    return;
                }

                /* next slot contains bucket 1 or more removed from it's optimal position: shift it back */
                *bucket_in_tbl = *next_bucket_in_tbl;
                bucket_in_tbl->psl--;
                next_bucket_in_tbl->psl = -1;

                /* step forward */
                bucket_in_tbl = next_bucket_in_tbl;
            }

            /* should never happen */
            assert(0 && "Bug in *_hash_table_remove");
        }

        /* go to next slot */
        index++;
        index &= (hash_table->n_buckets - 1);
    }

    return;
}

#define HGL_HASH_TABLE_FUNC_REMOVE_BY_COPY _CONCAT3(hgl_, HGL_HASH_TABLE_ID, _hash_table_remove_cp)
static inline void HGL_HASH_TABLE_FUNC_REMOVE_BY_COPY(HGL_HASH_TABLE_STRUCT *hash_table,
                                                      HGL_HASH_TABLE_KEY_TYPE key)
{
    HGL_HASH_TABLE_FUNC_REMOVE(hash_table, &key);
}
