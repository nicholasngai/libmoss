#ifndef LIBMOSS_DEFS_H
#define LIBMOSS_DEFS_H

#include <stdint.h>

/* A document. */
typedef struct moss_doc {
    int64_t id;
    const char *path;
} moss_doc_t;

/* A token entry for a document. */
typedef struct moss_token {
    uint64_t token;
    moss_doc_t *doc;
    unsigned long pos;
} moss_token_t;

/* A hash over several tokens. */
typedef struct moss_hash {
    uint64_t hash;
    moss_doc_t *doc;
    unsigned long start_pos;
    unsigned long end_pos;
} moss_hash_t;

#endif /* libmoss/defs. */
