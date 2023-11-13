#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define HGL_HASH_TABLE_KEY_TYPE int
#define HGL_HASH_TABLE_VALUE_TYPE int
#define HGL_HASH_TABLE_ID itoi
#include "hgl_hash_table.h"

#undef HGL_HASH_TABLE_KEY_TYPE
#undef HGL_HASH_TABLE_VALUE_TYPE
#undef HGL_HASH_TABLE_ID
#define HGL_HASH_TABLE_KEY_TYPE const char *
#define HGL_HASH_TABLE_VALUE_TYPE int
#define HGL_HASH_TABLE_ID strtoi
#undef HGL_HASH_TABLE_KEYSIZE_FUNC
#define HGL_HASH_TABLE_KEYSIZE_FUNC (strlen_keysize)
#undef HGL_HASH_TABLE_KEYCMP_FUNC
#define HGL_HASH_TABLE_KEYCMP_FUNC (strcmp_keycmp)
#include "hgl_hash_table.h"

int main(void)
{

    printf("hello\n");

#if 1
    printf("hello\n");

    hgl_itoi_hash_table_t hash_table;
    hgl_itoi_hash_table_init(&hash_table, 29);

    /* print hash table */
    printf("Hash table:\n");
    for (uint32_t i = 0; i < hash_table.n_buckets; i++) {
        hgl_itoi_bucket_t bucket = hash_table.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %d, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }

    hgl_itoi_hash_table_insert_cpkv(&hash_table, 1, 0);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 2, 1);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 3, 1);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 4, 2);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 5, 3);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 6, 5);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 7, 8);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 8, 13);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 9, 21);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 10, 34);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 11, 55);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 12, 89);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 13, 144);
    

    hgl_itoi_hash_table_insert_cpkv(&hash_table, 239, -1);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 15120, -34);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 1121, 5523);

    /* print hash table before resize */
    printf("Hash table:\n");
    for (uint32_t i = 0; i < hash_table.n_buckets; i++) {
        hgl_itoi_bucket_t bucket = hash_table.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %d, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }

    hgl_itoi_hash_table_insert_cpkv(&hash_table, -12312, 3389);
    hgl_itoi_hash_table_insert_cpkv(&hash_table, 124142143, 142);
    
    /* print hash table */
    printf("Hash table:\n");
    for (uint32_t i = 0; i < hash_table.n_buckets; i++) {
        hgl_itoi_bucket_t bucket = hash_table.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %d, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }

    int *value = hgl_itoi_hash_table_lookup_cp(&hash_table, 10);
    if (value != NULL){
        printf("looked up key = %d. found value = %d.\n", 10, *value);
    } else {
        printf("looked up key = %d. found NOTHING.\n", 10);
    }
    
    hgl_itoi_hash_table_remove_cp(&hash_table, 12333323); // remove key that doesn't exist
    hgl_itoi_hash_table_remove_cp(&hash_table, 2); // remove key that doesn't produce shift
    hgl_itoi_hash_table_remove_cp(&hash_table, 5); // remove key that does produce a couple shifts
    //hgl_itoi_hash_table_remove_by_value(&hash_table, 6); // remove key that does produce a couple shifts
    
    /* print hash table */
    printf("Hash table:\n");
    for (uint32_t i = 0; i < hash_table.n_buckets; i++) {
        hgl_itoi_bucket_t bucket = hash_table.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %d, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }
   
    hgl_itoi_hash_table_free(&hash_table);
#endif

#if 1
    printf("-------------------------------------------------------------------------\n");
    printf("---------------------------- string test --------------------------------\n");
    printf("-------------------------------------------------------------------------\n");
    hgl_strtoi_hash_table_t hash_table2;
    hgl_strtoi_hash_table_init(&hash_table2, 6); /* size becomes 8 */
    const char *str1 = "hello";
    const char *str2 = "goodbye";
    const char *str3 = "fish";
    const char *str4 = "bird";
    const char *str5 = "Hello, my name is Earl";
    const char *str6 = "Hello, my name is Crabman";
    const char *str7 = "Hello, my name is Randy";
    const char *str8 = "uhuh hello";
    const char *str9 = "kranko";
    const char *str10 = "franko";
    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str1, 1);
    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str2, 2);
    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str3, 4);
    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str4, 8);
    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str5, 16);
    

    /* print hash table before rehash */
    printf("Hash table 2:\n");
    printf("N buckets: %d, N occupied: %d, load factor: %f\n", 
           hash_table2.n_buckets, 
           hash_table2.n_occupied_buckets,
           (float)hash_table2.n_occupied_buckets / (float)hash_table2.n_buckets);
    for (uint32_t i = 0; i < hash_table2.n_buckets; i++) {
        hgl_strtoi_bucket_t bucket = hash_table2.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %s, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }


    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str6, 32);


    /* print hash table before rehash */
    printf("Hash table 2:\n");
    printf("N buckets: %d, N occupied: %d, load factor: %f\n", 
           hash_table2.n_buckets, 
           hash_table2.n_occupied_buckets,
           (float)hash_table2.n_occupied_buckets / (float)hash_table2.n_buckets);
    for (uint32_t i = 0; i < hash_table2.n_buckets; i++) {
        hgl_strtoi_bucket_t bucket = hash_table2.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %s, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }
    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str7, 64);
    
    /* print hash table before rehash */
    printf("Hash table 2:\n");
    printf("N buckets: %d, N occupied: %d, load factor: %f\n", 
           hash_table2.n_buckets, 
           hash_table2.n_occupied_buckets,
           (float)hash_table2.n_occupied_buckets / (float)hash_table2.n_buckets);
    for (uint32_t i = 0; i < hash_table2.n_buckets; i++) {
        hgl_strtoi_bucket_t bucket = hash_table2.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %s, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }
    
    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str8, 64);
    
    /* print hash table  */
    printf("Hash table 2:\n");
    printf("N buckets: %d, N occupied: %d, load factor: %f\n", 
           hash_table2.n_buckets, 
           hash_table2.n_occupied_buckets,
           (float)hash_table2.n_occupied_buckets / (float)hash_table2.n_buckets);
    for (uint32_t i = 0; i < hash_table2.n_buckets; i++) {
        hgl_strtoi_bucket_t bucket = hash_table2.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %s, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }

    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str9, 1337);
    hgl_strtoi_hash_table_insert_cpv(&hash_table2, &str10, 1338);
    printf("N buckets: %d, N occupied: %d, load factor: %f\n", 
           hash_table2.n_buckets, 
           hash_table2.n_occupied_buckets,
           (float)hash_table2.n_occupied_buckets / (float)hash_table2.n_buckets);
    for (uint32_t i = 0; i < hash_table2.n_buckets; i++) {
        hgl_strtoi_bucket_t bucket = hash_table2.tbl[i];
        if (bucket.psl == -1) {
            printf(" [%u] -- \n", i);
        } else {
            printf(" [%u] psl = %d, key = %s, value = %d\n", i, bucket.psl, bucket.key, bucket.value);
        }
    }
    
    value = hgl_strtoi_hash_table_lookup(&hash_table2, &str5);
    if (value != NULL){
        printf("looked up key = %s. found value = %d.\n", str5, *value);
    } else {
        printf("looked up key = %d. found NOTHING.\n", 10);
    }


#endif

}

