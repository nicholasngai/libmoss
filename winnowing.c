#include "libmoss/winnowing.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "libmoss/defs.h"
#include "libmoss/internal/defs.h"

int moss_winnow_init(moss_winnow_t *winnow, size_t w) {
    int ret;

    winnow->w = w;
    winnow->window = malloc(winnow->w * sizeof(*winnow->window));
    if (!winnow->window) {
        ret = errno;
        goto exit;
    }
    moss_winnow_reset(winnow);

    ret = 0;

exit:
    return ret;
}

void moss_winnow_reset(moss_winnow_t *winnow) {
    winnow->window_idx = 0;
    winnow->min_idx = 0;
    winnow->first_window_read = false;
}

void moss_winnow_free(moss_winnow_t *winnow) {
    free(winnow->window);
}

size_t moss_winnow_process(moss_winnow_t *restrict winnow,
        const moss_hash_t *restrict hashes, size_t len,
        moss_hash_t *restrict fingerprint) {
    size_t i = 0;

    if (!winnow->first_window_read) {
        size_t hashes_to_copy = MIN(winnow->w - winnow->window_idx, len);
        for (; i < hashes_to_copy; i++) {
            winnow->window[winnow->window_idx + i] = hashes[i];
            if (winnow->window[winnow->window_idx + i].hash
                    < winnow->window[winnow->min_idx].hash) {
                winnow->min_idx = winnow->window_idx + i;
            }
        }
        winnow->window_idx = (winnow->window_idx + hashes_to_copy) % winnow->w;
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
            size_t j = winnow->window_idx % winnow->w;
            do {
                if (winnow->window[j].hash
                        < winnow->window[winnow->min_idx].hash) {
                    winnow->min_idx = j;
                }
                j = (j - 1 + winnow->w) % winnow->w;
            } while (j != winnow->window_idx);
            winnow->window_idx = (winnow->window_idx + 1) % winnow->w;
            *fingerprint = winnow->window[winnow->min_idx];
            return i;
        }

        /* The previous min element is still in the window. Compare the new
         * element against the previous min instead of scanning the whole
         * window and return it if it's lower. */
        if (winnow->window[winnow->window_idx].hash
                < winnow->window[winnow->min_idx].hash) {
            winnow->min_idx = winnow->window_idx;
            winnow->window_idx = (winnow->window_idx + 1) % winnow->w;
            *fingerprint = winnow->window[winnow->min_idx];
            return i;
        }

        winnow->window_idx = (winnow->window_idx + 1) % winnow->w;
    }

    return 0;
}
