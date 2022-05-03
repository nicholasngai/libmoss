#include "libmoss/winnowing.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libmoss/internal/defs.h"

int moss_winnow_init(moss_winnow_t *winnow, size_t k) {
    int ret;

    winnow->k = k;
    winnow->window_idx = 0;
    winnow->min_idx = 0;
    winnow->first_window_read = false;
    winnow->window = malloc(k * sizeof(*winnow->window));
    if (!winnow->window) {
        ret = errno;
        goto exit;
    }

    ret = 0;

exit:
    return ret;
}

void moss_winnow_free(moss_winnow_t *winnow) {
    free(winnow->window);
}

size_t moss_winnow_process(moss_winnow_t *restrict winnow,
        const uint64_t *restrict hashes, size_t len,
        uint64_t *restrict fingerprint) {
    size_t i = 0;

    if (!winnow->first_window_read) {
        size_t hashes_to_copy = MIN(winnow->k - winnow->window_idx, len);
        for (; i < hashes_to_copy; i++) {
            winnow->window[winnow->window_idx + i] = hashes[i];
            if (winnow->window[winnow->window_idx + i]
                    < winnow->window[winnow->min_idx]) {
                winnow->min_idx = winnow->window_idx + i;
            }
        }
        winnow->window_idx = (winnow->window_idx + hashes_to_copy) % winnow->k;
        if (!winnow->window_idx) {
            /* We've fully read in the first window. */
            winnow->first_window_read = true;
            *fingerprint = winnow->window[winnow->min_idx];
            return i;
        }

        return 0;
    }

    while (i < len) {
        winnow->window[winnow->window_idx] = hashes[i];
        i++;

        if (winnow->window_idx == winnow->min_idx) {
            /* The previous min element just exited the window, so scan the
             * whole window again starting from the right to find the new
             * lowest element and return it. Favor the right-most element to
             * break ties. */
            size_t j = winnow->window_idx % winnow->k;
            do {
                if (winnow->window[j] < winnow->window[winnow->min_idx]) {
                    winnow->min_idx = j;
                }
                j = (j - 1 + winnow->k) % winnow->k;
            } while (j != winnow->window_idx);
            winnow->window_idx = (winnow->window_idx + 1) % winnow->k;
            *fingerprint = winnow->window[winnow->min_idx];
            return i;
        }

        /* The previous min element is still in the window. Compare the new
         * element against the previous min instead of scanning the whole
         * window and return it if it's lower. */
        if (winnow->window[winnow->window_idx]
                < winnow->window[winnow->min_idx]) {
            winnow->min_idx = winnow->window_idx;
            winnow->window_idx = (winnow->window_idx + 1) % winnow->k;
            *fingerprint = winnow->window[winnow->min_idx];
            return i;
        }

        winnow->window_idx = (winnow->window_idx + 1) % winnow->k;
    }

    return 0;
}
