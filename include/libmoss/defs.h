#ifndef LIBMOSS_DEFS_H
#define LIBMOSS_DEFS_H

#include <stdint.h>

/* A token entry for a document. */
typedef struct moss_token {
    uint64_t token;
    int64_t doc;
    unsigned long pos;
} moss_token_t;

/* A hash over several tokens. */
typedef struct moss_hash {
    uint64_t hash;
    int64_t doc;
    unsigned long start_pos;
    unsigned long end_pos;
} moss_hash_t;

#endif /* libmoss/defs. */
