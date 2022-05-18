#ifndef LIBMOSS_MOSS_H
#define LIBMOSS_MOSS_H

#include <stddef.h>
#include <stdint.h>
#include "libmoss/defs.h"
#include "libmoss/hashing.h"
#include "libmoss/internal/multimap.h"
#include "libmoss/winnowing.h"

/* An instance of MOSS to determine similarity across documents. */
typedef struct moss {
    /* Length of the k-grams. Matches below this length are not detected. */
    size_t k;

    /* Length of the window of hashes across which to winnow. Matches above
     * length W + K - 1 are guaranteed to be detected. */
    size_t w;

    /* Map of fingerprints to their documents. */
    struct moss_multimap fingerprints;

    /* The current document being fingerprinted. */
    int64_t doc;

    /* The hashing instance for the current document. */
    moss_hashing_t doc_hashing;

    /* The winnowing instance for the current document's hashes. */
    moss_winnow_t doc_winnow;

    /* Buffer to hold hashes output by the hashing context. */
    moss_hash_t *hashes_buf;
} moss_t;

/* Initializes the MOSS instance with the given parameters of K and W. */
int moss_init(moss_t *moss, size_t k, size_t w);

/* Frees the MOSS instance. */
void moss_free(moss_t *moss);

/* Feeds TOKENS into MOSS as the tokens for the DOC. If the DOC is the same as
 * the previous call to moss_input, TOKENS is assumed to be a continuation for
 * the tokens of the same document. Otherwise, DOC must be unique across all
 * calls to moss_input (i.e. it is undefined to input tokens for DOC == 1, then
 * DOC == 2, then DOC == 1). */
int moss_input(moss_t *moss, int64_t doc, const moss_token_t *tokens,
        size_t tokens_len);

#endif /* libmoss/moss.h */
