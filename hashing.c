#include "libmoss/hashing.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
    hashing->prev_tokens =
        malloc((hashing->k - 1) * sizeof(*hashing->prev_tokens));
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
}

int moss_hashing_get_hashes(moss_hashing_t *restrict hashing,
        uint64_t *restrict hashes, size_t hashes_len) {
    int ret = 0;

    while ((size_t) ret + hashing->k - 1 < hashing->input_len
            && (size_t) ret < hashes_len) {
        hashes[ret] = hash_djb2(hashing->input + ret, hashing->k);
        ret++;
    }
    hashing->input += ret;
    hashing->input_len -= ret;

    /* If we've reached the end of the input stream, then copy the last k - 1
     * tokens into the prev_tokens buffer to save them for next time. */
    if (hashing->input_len < hashing->k) {
        memcpy(hashing->prev_tokens, hashing->input,
                (hashing->k - 1) * sizeof(*hashing->prev_tokens));
    }

    return ret;
}
