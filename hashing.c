#include "libmoss/hashing.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libmoss/internal/defs.h"

struct hash_djb2 {
    uint64_t hash;
};

static int hash_djb2_init(struct hash_djb2 *djb2) {
    djb2->hash = 5381;
    return 0;
}

static int hash_djb2_feed(struct hash_djb2 *djb2, uint64_t token) {
    djb2->hash = (djb2->hash << 5) + djb2->hash + token;
    return 0;
}

static int hash_djb2_finalize(struct hash_djb2 *djb2, uint64_t *hash) {
    *hash = djb2->hash;
    return 0;
}

int moss_hashing_init(moss_hashing_t *hashing, size_t k) {
    int ret;

    hashing->k = k;
    hashing->prev_tokens =
        malloc((hashing->k - 1) * sizeof(*hashing->prev_tokens));
    if (!hashing->prev_tokens) {
        ret = errno;
        goto exit;
    }
    moss_hashing_reset(hashing);

    ret = 0;

exit:
    return ret;
}

void moss_hashing_reset(moss_hashing_t *hashing) {
    hashing->input_len = 0;
    hashing->prev_tokens_len = 0;
}

void moss_hashing_free(moss_hashing_t *hashing) {
    free(hashing->prev_tokens);
}

void moss_hashing_input_tokens(moss_hashing_t *restrict hashing,
        const uint64_t *restrict tokens, size_t len) {
    hashing->input = tokens;
    hashing->input_len = len;
}

int moss_hashing_get_hashes(moss_hashing_t *restrict hashing,
        uint64_t *restrict hashes, size_t hashes_len) {
    size_t total_tokens_read = 0;
    size_t tokens_read;
    int ret;

    /* Hash runs of tokens including prev_tokens. */
    tokens_read = 0;
    while (tokens_read < hashing->prev_tokens_len
            && tokens_read + hashing->k - hashing->prev_tokens_len - 1
                < hashing->input_len
            && total_tokens_read + total_tokens_read < hashes_len) {
        struct hash_djb2 hash;
        ret = hash_djb2_init(&hash);
        if (ret) {
            goto exit;
        }

        /* Hash first part of run from prev_tokens. */
        for (size_t i = tokens_read; i < hashing->prev_tokens_len; i++) {
            ret = hash_djb2_feed(&hash, hashing->prev_tokens[i]);
            if (ret) {
                goto exit;
            }
        }

        /* Hash second part of run from input. */
        for (size_t i = 0;
                i < hashing->k - (hashing->prev_tokens_len - tokens_read);
                i++) {
            ret = hash_djb2_feed(&hash, hashing->input[i]);
            if (ret) {
                goto exit;
            }
        }

        ret =
            hash_djb2_finalize(&hash,
                    &hashes[total_tokens_read + tokens_read]);
        if (ret) {
            goto exit;
        }

        tokens_read++;
    }
    memmove(hashing->prev_tokens, hashing->prev_tokens + tokens_read,
            (hashing->prev_tokens_len - tokens_read)
                * sizeof(*hashing->prev_tokens));
    hashing->prev_tokens_len -= tokens_read;
    total_tokens_read += tokens_read;

    /* Hash runs of tokens in the input stream until either the end of the
     * input is reached or the end of the output buffer is reached. */
    tokens_read = 0;
    while (tokens_read + hashing->k - 1 < hashing->input_len
            && total_tokens_read + tokens_read < hashes_len) {
        struct hash_djb2 hash;
        ret = hash_djb2_init(&hash);
        if (ret) {
            goto exit;
        }

        for (size_t i = tokens_read; i < tokens_read + hashing->k; i++) {
            ret = hash_djb2_feed(&hash, hashing->input[i]);
            if (ret) {
                goto exit;
            }
        }

        ret =
            hash_djb2_finalize(&hash,
                    &hashes[total_tokens_read + tokens_read]);
        if (ret) {
            goto exit;
        }

        tokens_read++;
    }
    hashing->input += tokens_read;
    hashing->input_len -= tokens_read;
    total_tokens_read += tokens_read;

    /* If we've reached the end of the input stream, then copy the last k - 1
     * tokens into the prev_tokens buffer to save them for next time. */
    if (hashing->input_len < hashing->k) {
        size_t tokens_to_copy =
            MIN(hashing->k - 1 - hashing->prev_tokens_len, hashing->input_len);
        memcpy(hashing->prev_tokens + hashing->prev_tokens_len,
                hashing->input + hashing->input_len - tokens_to_copy,
                tokens_to_copy * sizeof(*hashing->prev_tokens));
        hashing->prev_tokens_len += tokens_to_copy;
        hashing->input += tokens_to_copy;
        hashing->input_len -= tokens_to_copy;
    }

    ret = total_tokens_read;

exit:
    return ret;
}
