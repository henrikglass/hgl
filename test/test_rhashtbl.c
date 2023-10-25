#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define HGL_RHHASHTBL_KEY_TYPE int
#define HGL_RHHASHTBL_VALUE_TYPE int
#define HGL_RHHASHTBL_ID itoi
#include "hgl_rhhashtbl.h"

int main(void)
{
    printf("hello\n");

    hgl_itoi_rhhashtbl_t hash_table;
    hgl_itoi_rhhashtbl_init(&hash_table, 29);

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

    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 1, 0);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 2, 1);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 3, 1);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 4, 2);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 5, 3);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 6, 5);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 7, 8);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 8, 13);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 9, 21);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 10, 34);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 11, 55);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 12, 89);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 13, 144);
    

    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 239, -1);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 15120, -34);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 1121, 5523);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, -12312, 3389);
    hgl_itoi_rhhashtbl_insert_by_value(&hash_table, 124142143, 142);
    
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

    int *value = hgl_itoi_rhhashtbl_lookup_by_value(&hash_table, 10);
    if (value != NULL){
        printf("looked up key = %d. found value = %d.\n", 10, *value);
    } else {
        printf("looked up key = %d. found NOTHING.\n", 10);
    }
}
