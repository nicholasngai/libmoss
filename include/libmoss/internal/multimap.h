#ifndef LIBMOSS_INTERNAL_HASHMAP_H
#define LIBMOSS_INTERNAL_HASHMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <threads.h>

struct moss_multimap_bucket {
    uint64_t key;
    void **vals;
    size_t vals_len;
    size_t vals_cap;
    struct moss_multimap_bucket *next;
    mtx_t lock;
};

struct moss_multimap {
    struct moss_multimap_bucket **buckets;
    size_t num_buckets;
    mtx_t lock;
};

struct moss_multimap_iter {
    struct moss_multimap *multimap;
    struct moss_multimap_bucket *bucket;
};

int moss_multimap_init(struct moss_multimap *multimap, size_t num_buckets);
void moss_multimap_free(struct moss_multimap *multimap,
        void (*callback)(void *val));

bool moss_multimap_get(struct moss_multimap *multimap, uint64_t key,
        void ***result, size_t *result_len);
int moss_multimap_add(struct moss_multimap *multimap, uint64_t key, void *val);

struct moss_multimap_iter moss_multimap_iter_begin(
        struct moss_multimap *multimap);
void moss_multimap_iter_next(struct moss_multimap_iter *iter);

static inline bool moss_multimap_iter_finished(
        struct moss_multimap_iter *iter) {
    return iter->bucket;
}

#endif /* libmoss/internal/multimap.h */
