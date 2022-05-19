#include "libmoss/internal/multimap.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>

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
    ret = mtx_init(&multimap->lock, mtx_plain);
    if (ret) {
        goto exit;
    }

exit:
    return ret;
}

void moss_multimap_free(struct moss_multimap *multimap,
        void (*callback)(void *val)) {
    for (size_t i = 0; i < multimap->num_buckets; i++) {
        struct moss_multimap_bucket *bucket = multimap->buckets[i];
        while (bucket) {
            if (callback) {
                for (size_t i = 0; i < bucket->vals_len; i++) {
                    callback(bucket->vals[i]);
                }
            }
            free(bucket->vals);
            struct moss_multimap_bucket *next_bucket = bucket->next;
            free(bucket);
            bucket = next_bucket;
        }
    }
    free(multimap->buckets);
}

bool moss_multimap_get(struct moss_multimap *restrict multimap, uint64_t key,
        void ***restrict result, size_t *result_len) {
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
        void *val) {
    int ret;
    uint64_t index = key % multimap->num_buckets;
    mtx_t *cur_lock;

    ret = mtx_lock(&multimap->lock);
    if (ret) {
        goto exit;
    }
    cur_lock = &multimap->lock;

    struct moss_multimap_bucket **bucket = &multimap->buckets[index];
    while (*bucket && (*bucket)->key < key) {
        ret = mtx_lock(&(*bucket)->lock);
        if (ret) {
            goto exit;
        }
        ret = mtx_unlock(cur_lock);
        if (ret) {
            goto exit;
        }
        cur_lock = &(*bucket)->lock;

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
        new_bucket->vals_len = 0;
        new_bucket->vals_cap = INITIAL_BUCKET_LEN;
        new_bucket->next = *bucket;
        new_bucket->vals =
            malloc(INITIAL_BUCKET_LEN * sizeof(*new_bucket->vals));
        if (!new_bucket->vals) {
            free(new_bucket);
            ret = errno;
            goto exit;
        }
        ret = mtx_init(&new_bucket->lock, mtx_plain);
        if (ret) {
            goto exit;
        }
        *bucket = new_bucket;

        ret = mtx_lock(&new_bucket->lock);
        if (ret) {
            goto exit;
        }
        ret = mtx_unlock(cur_lock);
        if (ret) {
            goto exit;
        }
        cur_lock = &new_bucket->lock;
    }

    /* Resize the array if needed. */
    if ((*bucket)->vals_len == (*bucket)->vals_cap) {
        size_t new_cap = (*bucket)->vals_cap * 2;
        void **new_vals =
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

    ret = mtx_unlock(cur_lock);
    if (ret) {
        goto exit;
    }

    ret = 0;

exit:
    return ret;
}

struct moss_multimap_iter moss_multimap_iter_begin(
        struct moss_multimap *multimap) {
    struct moss_multimap_iter ret = {
        .multimap = multimap,
    };

    for (size_t i = 0; i < ret.multimap->num_buckets; i++) {
        if (ret.multimap->buckets[i]) {
            ret.bucket = ret.multimap->buckets[i];
            goto exit;
        }
    }

    ret.bucket = NULL;

exit:
    return ret;
}

void moss_multimap_iter_next(struct moss_multimap_iter *iter) {
    /* Check if the bucket has any next items. */
    if (iter->bucket->next) {
        iter->bucket = iter->bucket->next;
        return;
    }

    /* Iterate over future hashes. */
    for (size_t i = iter->bucket->key % iter->multimap->num_buckets + 1;
            i < iter->multimap->num_buckets; i++) {
        if (iter->multimap->buckets[i]) {
            iter->bucket = iter->multimap->buckets[i];
            return;
        }
    }

    /* Iteration finished. */
    iter->bucket = NULL;
}
