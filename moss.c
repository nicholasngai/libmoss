#include "libmoss/moss.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include "libmoss/hashing.h"
#include "libmoss/internal/multimap.h"
#include "libmoss/winnowing.h"

#define FINGERPRINTS_MAP_SIZE 1048576
#define HASHES_BUF_LEN 4096

int moss_init(moss_t *moss, size_t k, size_t w) {
    int ret;

    moss->k = k;
    moss->w = w;
    moss->doc = NULL;
    ret = moss_multimap_init(&moss->fingerprints, FINGERPRINTS_MAP_SIZE);
    if (ret) {
        goto exit;
    }
    ret = moss_hashing_init(&moss->default_hashing, moss->k);
    if (ret) {
        goto exit_free_fingerprints;
    }
    ret = moss_winnow_init(&moss->default_winnow, moss->w);
    if (ret) {
        goto exit_free_hashing;
    }

    return 0;

exit_free_hashing:
    moss_hashing_free(&moss->default_hashing);
exit_free_fingerprints:
    moss_multimap_free(&moss->fingerprints, NULL);
exit:
    return ret;
}

void moss_free(moss_t *moss) {
    moss_multimap_free(&moss->fingerprints, (void (*)(void *)) free);
    moss_hashing_free(&moss->default_hashing);
    moss_winnow_free(&moss->default_winnow);
}

int moss_input(moss_t *moss, moss_doc_t *doc, const moss_token_t *tokens,
        size_t tokens_len) {
    if (moss->doc != doc) {
        moss_hashing_reset(&moss->default_hashing);
        moss_winnow_reset(&moss->default_winnow);
        moss->doc = doc;
    }
    return moss_input_threaded(moss, &moss->default_hashing,
            &moss->default_winnow, tokens, tokens_len);
}

int moss_input_threaded(moss_t *moss, moss_hashing_t *hashing,
        moss_winnow_t *winnow, const moss_token_t *tokens, size_t tokens_len) {
    int ret;

    /* Input tokens to hashing. */
    moss_hashing_input_tokens(hashing, tokens, tokens_len);

    size_t hashes_read;
    do {
        /* Get some hashes. */
        moss_hash_t hashes_buf[HASHES_BUF_LEN];
        ret = moss_hashing_get_hashes(hashing, hashes_buf, HASHES_BUF_LEN);
        if (ret < 0) {
            goto exit;
        }
        hashes_read = ret;

        /* Winnow the hashes and feed each one to the hash map. */
        size_t hashes_winnowed = 0;
        while (ret) {
            moss_hash_t fingerprint;
            ret =
                moss_winnow_process(winnow, hashes_buf + hashes_winnowed,
                        hashes_read - hashes_winnowed, &fingerprint);
            hashes_winnowed += ret;

            if (!ret) {
                break;
            }

            moss_hash_t *entry = malloc(sizeof(*entry));
            if (!entry) {
                ret = errno;
                goto exit;
            }
            *entry = fingerprint;

            ret = moss_multimap_add(&moss->fingerprints, entry->hash, entry);
            if (ret == -1) {
                free(entry);
                goto exit;
            }
        }
    } while (hashes_read);

exit:
    return ret;
}
