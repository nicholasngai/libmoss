#ifndef LIBMOSS_HASHING_H
#define LIBMOSS_HASHING_H

#include <stddef.h>
#include <stdint.h>

typedef struct moss_hashing {
    /* Length of the k-grams to hash. */
    size_t k;

    /* The current token input buffer. */
    const uint64_t *input;
    size_t input_len;

    /* The previously K - 1 tokens in the stream. Used to resume k-grams when a
     * new stream of tokens is fed in. */
    uint64_t *prev_tokens;
} moss_hashing_t;

/* Initializes the hashing context to read k-grams of length K from the token
 * input stream. */
int moss_hashing_init(moss_hashing_t *hashing, size_t k);

/* Frees the hashing context. */
void moss_hashing_free(moss_hashing_t *hashing);

/* Feeds the TOKENS of length LEN into the hashing context. It is undefined to
 * call this function if there is an unconsumed token buffer. TOKENS must not
 * be modified until moss_hashing_get_hashes returns 0. */
void moss_hashing_input_tokens(moss_hashing_t *hashing, const uint64_t *tokens,
        size_t len);

/* Gets hashes of k-grams of the input tokens and writes them into HASHES, a
 * buffer of length HASHES_LEN. Returns the number of hashes written or zero if
 * the end of either the input buffer or output buffer is reached. */
int moss_hashing_get_hashes(moss_hashing_t *hashing, uint64_t *hashes,
        size_t hashes_len);

#endif /* libmoss/hashing.h */
