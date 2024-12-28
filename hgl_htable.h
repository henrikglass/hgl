
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
 * hgl_htable.h implements a simple to use robin hood hash table. 
 *
 *
 * USAGE:
 *
 * Include hgl_htable.h file like this:
 *
 *     #define HGL_HTABLE_IMPLEMENTATION
 *     #include "hgl_htable.h"
 *
 * You can redefine the load factor threshold, hash seed, and the default allocator:
 *
 *     HGL_HTABLE_LOAD_FACTOR_THRESH     (default: 0.5f)
 *     HGL_HTABLE_HASH_SEED              (default: 0x1337)
 *     HGL_HTABLE_ALLOC                  (default: malloc)
 *     HGL_HTABLE_FREE                   (default: free)
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
 *     HglHashTable occupations = hgl_htable_make(djb2,    // hash function ptr
 *                                                keysize, // keysize function ptr
 *                                                keycmp,  // keycmp function ptr
 *                                                4096,    // initial size
 *                                                false);  // growable? (may resize above user-specified threshold)
 *     if (occupations.buckets == NULL) {
 *         return 1; // error 
 *     }
 *
 *     hgl_htable_insert(&occupations, "John", "Carpenter");
 *     hgl_htable_insert(&occupations, "Frank", "Plumber");
 *     hgl_htable_insert(&occupations, "Phil", "Electrician");
 *     hgl_htable_insert(&occupations, "Scott", "Regional Manager");
 *     hgl_htable_insert(&occupations, "Dwight", "Assistant Regional Manager");
 *
 *     // Fire John
 *     hgl_htable_remove(&occupations, "John");
 *  
 *     // Update Dwight's occupation title. Note: You don't actually have to remove "Dwight", you 
 *     // can actually just do insert again on his name without calling remove before.
 *     hgl_htable_remove(&occupations, "Dwight"); // Not necessary.
 *     hgl_htable_insert(&occupations, "Dwight", "Assistant (to the) Regional Manager");
 *
 *     hgl_htable_destroy(&occupations);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_HTABLE_H
#define HGL_HTABLE_H

/*--- robin hood hash table-specific macros ---------------------------------------------*/

/* CONFIGURABLE: HGL_HTABLE_ALLOC, HGL_HTABLE_FREE */
#if !defined(HGL_HTABLE_ALLOC) && \
    !defined(HGL_HTABLE_FREE)
#include <stdlib.h>
#define HGL_HTABLE_ALLOC  malloc
#define HGL_HTABLE_FREE   free
#endif

/* CONFIGURABLE: HGL_HTABLE_LOAD_FACTOR_THRESH */
#ifndef HGL_HTABLE_LOAD_FACTOR_THRESH
#define HGL_HTABLE_LOAD_FACTOR_THRESH 0.5f
#endif

/* CONFIGURABLE: HGL_HTABLE_HASH_SEED */
#ifndef HGL_HTABLE_HASH_SEED
#define HGL_HTABLE_HASH_SEED 0x1337
#endif

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    struct HglHashTableBucket *buckets;
    uint32_t n_occupied_buckets;
    uint32_t capacity;
    bool growable;
    void   (*hash)(const void *, size_t, uint32_t, uint32_t *);
    size_t (*keysize)(const void *);
    int    (*keycmp)(const void *, const void *);
} HglHashTable;

static inline HglHashTable hgl_htable_make(void (*hash_func)(const void *, size_t, uint32_t, uint32_t *),
                                           size_t (*keysize_func)(const void *),
                                           int (*keycmp_func)(const void *, const void *),
                                           uint32_t initial_capacity,
                                           bool growable);
static inline void hgl_htable_insert(HglHashTable *htbl, void *key, void *value);
static inline void hgl_htable_join(HglHashTable *htbl, HglHashTable *other);
static inline void *hgl_htable_lookup(HglHashTable *htbl, void *key);
static inline void hgl_htable_remove(HglHashTable *htbl, void *key);
static inline void hgl_htable_destroy(HglHashTable *htbl);

#endif

#ifdef HGL_HTABLE_IMPLEMENTATION

#include <stdio.h> // fprintf
#include <assert.h>

typedef struct HglHashTableBucket
{
    void *key;
    void *value;
    int8_t psl;
} HglHashTableBucket;

static inline void hgl_htable_grow_and_rehash_(HglHashTable *htbl)
{
    //printf("GROW!\n");
    assert((htbl->capacity & 0x80000000) == 0);

    /* construct a new table of double the previous size */
    htbl->capacity <<= 1;
    HglHashTableBucket *old_table = htbl->buckets;
    htbl->buckets = HGL_HTABLE_ALLOC(htbl->capacity * sizeof(HglHashTableBucket));
    htbl->n_occupied_buckets = 0;
    if (htbl->buckets == NULL) {
        fprintf(stderr, "[hgl_htable_grow_and_rehash_] Error: Allocation of new table failed. Aborting...\n");
        abort();
    }

    /* init all psls to -1 (free slot) */
    for (uint32_t i = 0; i < htbl->capacity; i++) {
        htbl->buckets[i].psl = -1;
    }

    /* iterate old hash table, rehash keys, and put into new table */
    uint32_t old_table_size = htbl->capacity >> 1;
    for (uint32_t i = 0; i < old_table_size; i++) {

        HglHashTableBucket *old_bucket = &old_table[i];

        /* Bucket unoccupied: skip */
        if (old_bucket->psl == -1) {
            continue;
        }

        hgl_htable_insert(htbl, old_bucket->key, old_bucket->value);
    }

    /* free old table */
    HGL_HTABLE_FREE(old_table);
}

/*--- Channel functions -----------------------------------------------------------------*/

static inline HglHashTable hgl_htable_make(void (*hash_func)(const void *, size_t, uint32_t, uint32_t *),
                                           size_t (*keysize_func)(const void *),
                                           int (*keycmp_func)(const void *, const void *),
                                           uint32_t initial_capacity,
                                           bool growable)
{

    if (hash_func == NULL) {
        fprintf(stderr, "[hgl_htable_make] Error: `hash_func` must not be NULL.\n");
    }
    if (keysize_func == NULL) {
        fprintf(stderr, "[hgl_htable_make] Error: `keysize_func` must not be NULL.\n");
    }
    if (keycmp_func == NULL) {
        fprintf(stderr, "[hgl_htable_make] Error: `keycmp_func` must not be NULL.\n");
    }
    if ((initial_capacity & (initial_capacity - 1)) != 0) {
        fprintf(stderr, "[hgl_htable_make] Error: `initial_capacity` must be a power of 2.\n");
    }

    HglHashTable htbl = {
        .buckets            = HGL_HTABLE_ALLOC(initial_capacity * sizeof(HglHashTableBucket)),
        .n_occupied_buckets = 0,
        .capacity           = initial_capacity,
        .growable           = growable,
        .hash               = hash_func,
        .keysize            = keysize_func,
        .keycmp             = keycmp_func,
    };

    if (htbl.buckets == NULL) {
        fprintf(stderr, "Allocation failed. Buy more RAM. <%s, %d>\n", __FILE__, __LINE__);
        return htbl;
    }

    /* init all psls to -1 (free slot) */
    for (uint32_t i = 0; i < htbl.capacity; i++) {
        htbl.buckets[i].psl = -1;
    }

    return htbl;
}

static inline void hgl_htable_destroy(HglHashTable *htbl)
{
    HGL_HTABLE_FREE(htbl->buckets);
}

static inline void hgl_htable_insert(HglHashTable *htbl, void *key, void *value)
{
    /* rehash if needed */
    const float EPSILON = 0.0001f;
    float load_factor = (float) htbl->n_occupied_buckets / (float) htbl->capacity;
    if (htbl->growable && (load_factor > (HGL_HTABLE_LOAD_FACTOR_THRESH - EPSILON))) {
        hgl_htable_grow_and_rehash_(htbl);
    }

    /* compute expected index of key */
    uint32_t hash;
    uint32_t index;
    htbl->hash(key, htbl->keysize(key), HGL_HTABLE_HASH_SEED, &hash);
    index = hash & (htbl->capacity - 1);

    //printf("inserting %s at %u?\n", (char *) key, index);

    /* construct bucket */
    HglHashTableBucket temp;
    HglHashTableBucket bucket_to_insert = {
        .psl   = 0,
        .key   = key,
        .value = value
    };

    /* attempt to insert bucket */
    for (uint32_t i = 0; i < htbl->capacity; i++) {

        // TODO make max probe sequence length smaller and rehash if we go over the limit.
        // Do benchmarks to find a good limit.

        /* get the bucket at `index` in the table */
        HglHashTableBucket *bucket_in_tbl = &htbl->buckets[index];

        /* bucket is free: insert */
        if (bucket_in_tbl->psl == -1) {
            *bucket_in_tbl = bucket_to_insert;
            htbl->n_occupied_buckets++;
            return;
        }

        /* key collision: replace */
        if (htbl->keycmp(bucket_in_tbl->key, bucket_to_insert.key) == 0) {
            *bucket_in_tbl = bucket_to_insert;
            return;
        }

        /* take from the rich, i.e. swap buckets */
        if (bucket_in_tbl->psl < bucket_to_insert.psl) {
            temp = bucket_to_insert;
            bucket_to_insert = *bucket_in_tbl;
            *bucket_in_tbl = temp;
        }

        /* go to next slot and increment PSL */
        index++;
        index &= (htbl->capacity - 1);
        bucket_to_insert.psl++;
    }

    fprintf(stderr, "[hgl_htable_insert] Hash table is full. Aborting...\n");
    abort();
}

static inline void hgl_htable_join(HglHashTable *htbl, HglHashTable *other)
{
    for (uint32_t i = 0; i < other->capacity; i++) {
        HglHashTableBucket *bucket = &other->buckets[i];
        if (bucket->psl != -1) {
            hgl_htable_insert(htbl, bucket->key, bucket->value);
        }
    }
}

static inline void *hgl_htable_lookup(HglHashTable *htbl, void *key)
{
    /* compute expected index of key */
    uint32_t hash;
    uint32_t index;
    htbl->hash(key, htbl->keysize(key), HGL_HTABLE_HASH_SEED, &hash);
    index = hash & (htbl->capacity - 1);

    /* perform lookup */
    for (int8_t i = 0; i < 127; i++) {

        // TODO make max probe sequence length smaller and rehash if we go over the limit.
        // Do benchmarks to find a good limit.

        /* get the bucket at `index` in the table */
        HglHashTableBucket *bucket = &htbl->buckets[index];

        /* bucket is free or bucket has PSL higher than what the sought after key
         * would have in that slot: no key exists. return NULL */
        if (bucket->psl == -1 || bucket->psl < i) {
            return NULL;
        }

        /* key found: return it */
        if (htbl->keycmp(key, bucket->key) == 0) {
            return bucket->value;
        }

        /* go to next slot */
        index++;
        index &= (htbl->capacity - 1);
    }

    return NULL;
}

static inline void hgl_htable_remove(HglHashTable *htbl, void *key)
{
    /* compute expected index of key */
    uint32_t hash;
    uint32_t index;
    htbl->hash(key, htbl->keysize(key), HGL_HTABLE_HASH_SEED, &hash);
    index = hash & (htbl->capacity - 1);

    /* perform lookup */
    for (int8_t i = 0; i < 127; i++) {

        // TODO make max probe sequence length smaller and rehash if we go over the limit.
        // Do benchmarks to find a good limit.

        /* get the bucket at `index` in the table */
        HglHashTableBucket *bucket_in_tbl = &htbl->buckets[index];

        /* bucket is free or bucket has PSL higher than what the sought after key
         * would have in that slot: no key exists. return */
        if (bucket_in_tbl->psl == -1 || bucket_in_tbl->psl < i) {
            return;
        }

        /* key found: remove it and backward shift nearby buckets if possible */
        if (htbl->keycmp(key, bucket_in_tbl->key) == 0) {
            /* set as unoccupied */
            bucket_in_tbl->psl = -1;

            /* backward shift until empty slot or bucket with PSL = 0 */
            for (uint32_t j = 0; j < htbl->capacity; j++) {
                HglHashTableBucket *next_bucket_in_tbl = &htbl->buckets[(index + j + 1) & (htbl->capacity - 1)];

                /* next slot is empty (psl = -1) or contains bucket with optimal position (psl = 0): return */
                if (next_bucket_in_tbl->psl < 1) {
                    htbl->n_occupied_buckets--;
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
            assert(0 && "Bug in hgl_htable_remove");
        }

        /* go to next slot */
        index++;
        index &= (htbl->capacity - 1);
    }

    return;
}

#endif
