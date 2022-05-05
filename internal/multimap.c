#include "libmoss/internal/multimap.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define INITIAL_BUCKET_LEN 8

int moss_multimap_init(struct moss_multimap *multimap, size_t num_buckets) {
    int ret;

    multimap->num_buckets = num_buckets;
    multimap->buckets =
        calloc(multimap->num_buckets, sizeof(struct moss_multimap_bucket **));
    if (!multimap->buckets) {
        ret = errno;
        goto exit;
    }

    ret = 0;

exit:
    return ret;
}

void moss_multimap_free(struct moss_multimap *multimap) {
    for (size_t i = 0; i < multimap->num_buckets; i++) {
        struct moss_multimap_bucket *bucket = multimap->buckets[i];
        while (bucket) {
            struct moss_multimap_bucket *next_bucket = bucket->next;
            free(bucket);
            bucket = next_bucket;
        }
    }
    free(multimap->buckets);
}

bool moss_multimap_get(struct moss_multimap *restrict multimap, uint64_t key,
        const void ***restrict result, size_t *result_len) {
    uint64_t index = key % multimap->num_buckets;

    struct moss_multimap_bucket *bucket = multimap->buckets[index];
    while (bucket && bucket->key < key) {
        bucket = bucket->next;
    }

    if (bucket && bucket->key == key) {
        if (result) {
            *result = bucket->vals;
        }
        if (result_len) {
            *result_len = bucket->vals_len;
        }
        return true;
    }
    return false;
}

int moss_multimap_add(struct moss_multimap *restrict multimap, uint64_t key,
        const void *val) {
    int ret;
    uint64_t index = key % multimap->num_buckets;

    struct moss_multimap_bucket **bucket = &multimap->buckets[index];
    while (*bucket && (*bucket)->key < key) {
        bucket = &(*bucket)->next;
    }

    /* Allocate a new bucket if needed. */
    if (!*bucket || (*bucket)->key > key) {
        struct moss_multimap_bucket *new_bucket =
            malloc(sizeof(*new_bucket));
        if (!new_bucket) {
            ret = errno;
            goto exit;
        }
        new_bucket->key = key;
        new_bucket->vals =
            malloc(INITIAL_BUCKET_LEN * sizeof(*new_bucket->vals));
        new_bucket->vals_len = 0;
        new_bucket->vals_cap = INITIAL_BUCKET_LEN;
        *bucket = new_bucket;
    }

    /* Resize the array if needed. */
    if ((*bucket)->vals_len == (*bucket)->vals_cap) {
        size_t new_cap = (*bucket)->vals_cap * 2;
        const void **new_vals =
            realloc((*bucket)->vals, new_cap * sizeof(*(*bucket)->vals));
        if (!new_vals) {
            ret = errno;
            goto exit;
        }
        (*bucket)->vals = new_vals;
        (*bucket)->vals_cap = new_cap;
    }

    /* Add value to array. */
    (*bucket)->vals_len++;
    (*bucket)->vals[(*bucket)->vals_len - 1] = val;

    ret = 0;

exit:
    return ret;
}
