#include "libmoss/internal/hashmap.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int moss_hashmap_init(struct moss_hashmap *hashmap, size_t num_buckets) {
    int ret;

    hashmap->num_buckets = num_buckets;
    hashmap->buckets =
        calloc(hashmap->num_buckets, sizeof(struct moss_hashmap_bucket **));
    if (!hashmap->buckets) {
        ret = errno;
        goto exit;
    }

    ret = 0;

exit:
    return ret;
}

void moss_hashmap_free(struct moss_hashmap *hashmap) {
    for (size_t i = 0; i < hashmap->num_buckets; i++) {
        struct moss_hashmap_bucket *bucket = hashmap->buckets[i];
        while (bucket) {
            struct moss_hashmap_bucket *next_bucket = bucket->next;
            free(bucket);
            bucket = next_bucket;
        }
    }
    free(hashmap->buckets);
}

bool moss_hashmap_get(struct moss_hashmap *restrict hashmap, uint64_t key,
        const void **restrict result) {
    uint64_t index = key % hashmap->num_buckets;

    struct moss_hashmap_bucket *bucket = hashmap->buckets[index];
    while (bucket && bucket->key < key) {
        bucket = bucket->next;
    }

    if (bucket && bucket->key == key) {
        if (result) {
            *result = bucket->val;
        }
        return true;
    }
    return false;
}

int moss_hashmap_put(struct moss_hashmap *restrict hashmap, uint64_t key,
        const void *val, const void **restrict old_val) {
    int ret;
    uint64_t index = key % hashmap->num_buckets;

    struct moss_hashmap_bucket **bucket = &hashmap->buckets[index];
    while (*bucket && (*bucket)->key < key) {
        bucket = &(*bucket)->next;
    }

    if (*bucket && (*bucket)->key == key) {
        if (old_val) {
            *old_val = (*bucket)->val;
        }
        (*bucket)->val = val;
        ret = 1;
    } else {
        struct moss_hashmap_bucket *new_bucket =
            malloc(sizeof(*new_bucket));
        if (!new_bucket) {
            ret = errno;
            goto exit;
        }
        new_bucket->key = key;
        new_bucket->val = val;
        new_bucket->next = *bucket;
        *bucket = new_bucket;
        ret = 0;
    }

exit:
    return ret;
}

bool moss_hashmap_delete(struct moss_hashmap *hashmap, uint64_t key,
        const void **old_val) {
    uint64_t index = key % hashmap->num_buckets;

    struct moss_hashmap_bucket **bucket = &hashmap->buckets[index];
    while (*bucket && (*bucket)->key < key) {
        bucket = &(*bucket)->next;
    }

    if (*bucket && (*bucket)->key == key) {
        if (old_val) {
            *old_val = (*bucket)->val;
        }
        struct moss_hashmap_bucket *old_bucket = *bucket;
        *bucket = (*bucket)->next;
        free(old_bucket);
        return true;
    }
    return false;
}
