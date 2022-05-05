#include "libmoss/hashing.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libmoss/internal/defs.h"

static uint64_t hash_djb2(const uint64_t *tokens, size_t len) {
    uint64_t hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = (hash << 5) + hash + tokens[i];
    }
    return hash;
}

int moss_hashing_init(moss_hashing_t *hashing, size_t k) {
    int ret;

    hashing->k = k;
    hashing->input_len = 0;
    hashing->prev_tokens_len = 0;
    hashing->prev_tokens =
        malloc((hashing->k * 2 - 2) * sizeof(*hashing->prev_tokens));
    if (!hashing->prev_tokens) {
        ret = errno;
        goto exit;
    }

    ret = 0;

exit:
    return ret;
}

void moss_hashing_free(moss_hashing_t *hashing) {
    free(hashing->prev_tokens);
}

void moss_hashing_input_tokens(moss_hashing_t *restrict hashing,
        const uint64_t *restrict tokens, size_t len) {
    hashing->input = tokens;
    hashing->input_len = len;

    /* If there are any tokens in prev_tokens, we need to bring k - 1 tokens
     * into the prev_tokens buffer for hashing to work properly. */
    if (hashing->prev_tokens_len) {
        size_t tokens_to_copy = MIN(hashing->input_len, hashing->k - 1);
        memcpy(hashing->prev_tokens + hashing->prev_tokens_len, tokens,
                tokens_to_copy * sizeof(*hashing->prev_tokens));
        hashing->prev_tokens_len += tokens_to_copy;
    }
}

int moss_hashing_get_hashes(moss_hashing_t *restrict hashing,
        uint64_t *restrict hashes, size_t hashes_len) {
    int ret = 0;
    size_t tokens_read = 0;

    /* Hash tokens in prev_tokens until its length < k (or the end of the
     * output buffer is reached. After this, all tokens from the previous
     * stream have been consumed. */
    while (tokens_read + hashing->k - 1 < hashing->prev_tokens_len
            && (size_t) ret + tokens_read < hashes_len) {
        hashes[ret + tokens_read] =
            hash_djb2(hashing->prev_tokens + tokens_read, hashing->k);
        tokens_read++;
    }
    if (tokens_read) {
        memmove(hashing->prev_tokens, hashing->prev_tokens + tokens_read,
                (hashing->prev_tokens_len - tokens_read)
                    * sizeof(*hashing->prev_tokens));
        hashing->prev_tokens_len -= tokens_read;
    }
    ret += tokens_read;

    /* Hash tokens in the input stream until either the end of the input is
     * reached or the end of the output buffer is reached. */
    tokens_read = 0;
    while (tokens_read + hashing->k - 1 < hashing->input_len
            && (size_t) ret + tokens_read < hashes_len) {
        hashes[ret + tokens_read] =
            hash_djb2(hashing->input + tokens_read, hashing->k);
        tokens_read++;
    }
    hashing->input += tokens_read;
    hashing->input_len -= tokens_read;
    ret += tokens_read;

    /* If we've reached the end of the input stream, then copy the last k - 1
     * tokens into the prev_tokens buffer to save them for next time. */
    if (hashing->prev_tokens_len < hashing->k
            && hashing->input_len < hashing->k) {
        /* It's possible that the input is of length < k - 1, which would mean
         * we need tokens that are still in prev_tokens. This memmove moves as
         * many required tokens as needed to the beginning. */
        size_t prev_tokens_needed =
            MIN(hashing->k - 1 - hashing->input_len, hashing->prev_tokens_len);
        memmove(hashing->prev_tokens,
                hashing->prev_tokens
                    + hashing->prev_tokens_len - prev_tokens_needed,
                prev_tokens_needed * sizeof(*hashing->prev_tokens));

        /* Copy remaining tokens into the end of that buffer. */
        memcpy(hashing->prev_tokens + prev_tokens_needed,
                hashing->input,
                hashing->input_len * sizeof(*hashing->prev_tokens));

        hashing->prev_tokens_len = prev_tokens_needed + hashing->input_len;
    }

    return ret;
}
