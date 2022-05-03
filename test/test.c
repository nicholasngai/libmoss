#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <libmoss/winnowing.h>

int main(void) {
    uint64_t hashes[] = {77, 74, 42, 17, 98, 50, 17, 98, 8, 88, 67, 39, 77, 74, 42, 17, 98};
    moss_winnow_t winnow;
    int ret;

    ret = moss_winnow_init(&winnow, 4);
    if (ret) {
        goto exit;
    }

    size_t hashes_read;
    size_t start = 0;
    do {
        uint64_t fingerprint;
        hashes_read =
            moss_winnow_process(&winnow, hashes + start,
                sizeof(hashes) / sizeof(*hashes) - start, &fingerprint);
        start += hashes_read;
        if (hashes_read) {
            printf("%lu %lu\n", start, fingerprint);
        }
    } while (hashes_read);

    moss_winnow_free(&winnow);

exit:
    return ret;
}
