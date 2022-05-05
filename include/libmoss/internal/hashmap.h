#ifndef LIBMOSS_INTERNAL_HASHMAP_H
#define LIBMOSS_INTERNAL_HASHMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct moss_hashmap_bucket {
    uint64_t key;
    const void *val;
    struct moss_hashmap_bucket *next;
};

struct moss_hashmap {
    struct moss_hashmap_bucket **buckets;
    size_t num_buckets;
};

int moss_hashmap_init(struct moss_hashmap *hashmap, size_t num_buckets);
void moss_hashmap_free(struct moss_hashmap *hashmap);

bool moss_hashmap_get(struct moss_hashmap *hashmap, uint64_t key,
        const void **val);
int moss_hashmap_put(struct moss_hashmap *hashmap, uint64_t key,
        const void *val, const void **old_val);
bool moss_hashmap_delete(struct moss_hashmap *hashmap, uint64_t key,
        const void **old_val);

#endif /* libmoss/internal/hashmap.h */
