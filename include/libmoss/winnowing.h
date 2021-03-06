#ifndef LIBMOSS_WINNOWING_H
#define LIBMOSS_WINNOWING_H

#include <stdbool.h>
#include <stddef.h>
#include "libmoss/defs.h"

typedef struct moss_winnow {
    /* Size of the window. */
    size_t w;
    /* Current window. This is a circular buffer that starts at
     * WINDOW[WINDOW_IDX] and ends at WINDOW[WINDOW_IDX - 1 % W] */
    moss_hash_t *window;
    /* Current window start position. */
    size_t window_idx;
    /* Current window minimum hash index. */
    size_t min_idx;
    /* Whether the first window has been read yet. */
    bool first_window_read;
} moss_winnow_t;

/* Initializes a winnowing context for windows of length W. */
int moss_winnow_init(moss_winnow_t *winnow, size_t w);

/* Resets a winnowing context. The value of w remains constant. */
void moss_winnow_reset(moss_winnow_t *winnow);

/* Frees a winnowing context. */
void moss_winnow_free(moss_winnow_t *winnow);

/* Performs winnowing on the array HASHES of length LEN and returns the next
 * fingerprint value in *FINGERPRINT. If the return value is zero, winnowing
 * completed on the entire array without returning a new value in *FINGERPRINT.
 * If the return value is non-zero and less than LEN, the value is the index of
 * the first hash value not yet read into the windowa, nd the next call to
 * winnow should start at &HASHES[ret] and have length LEN - ret. */
size_t moss_winnow_process(moss_winnow_t *winnow, const moss_hash_t *hashes,
        size_t len, moss_hash_t *fingerprint);

#endif /* libmoss/winnowing.h */
