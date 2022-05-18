#ifndef LIBMOSS_HASHING_H
#define LIBMOSS_HASHING_H

#include <stddef.h>
#include "libmoss/defs.h"

typedef struct moss_hashing {
    /* Length of the k-grams to hash. */
    size_t k;

    /* The current token input buffer. */
    const moss_token_t *input;
    size_t input_len;

    /* The last K - 1 tokens of the previous stream. Used to resume k-grams
     * when a new stream of tokens is fed in. This buffer is of length 2K - 2
     * so that the first K - 1 tokens in the next stream can placed adjacent to
     * the previous tokens for hashing purposes. The actual number of active
     * tokens in the buffer is PREV_TOKENS_LEN. If PREV_TOKENS_LEN < K, the
     * buffer should no longer be used. */
    moss_token_t *prev_tokens;
    size_t prev_tokens_len;
} moss_hashing_t;

/* Initializes the hashing context to read k-grams of length K from the token
 * input stream. */
int moss_hashing_init(moss_hashing_t *hashing, size_t k);

/* Resets a hashing context. The value of k remains constant. */
void moss_hashing_reset(moss_hashing_t *hashing);

/* Frees the hashing context. */
void moss_hashing_free(moss_hashing_t *hashing);

/* Feeds the TOKENS of length LEN into the hashing context. It is undefined to
 * call this function if there is an unconsumed token buffer. TOKENS must not
 * be modified until moss_hashing_get_hashes returns 0. */
void moss_hashing_input_tokens(moss_hashing_t *hashing,
        const moss_token_t *tokens, size_t len);

/* Gets hashes of k-grams of the input tokens and writes them into HASHES, a
 * buffer of length HASHES_LEN. Returns the number of hashes written or zero if
 * the end of either the input buffer or output buffer is reached. */
int moss_hashing_get_hashes(moss_hashing_t *hashing, moss_hash_t *hashes,
        size_t hashes_len);

#endif /* libmoss/hashing.h */
