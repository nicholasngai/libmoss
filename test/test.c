#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <libmoss/winnowing.h>

int main(void) {
    uint64_t hashes[] = {77, 74, 42, 17, 98, 50, 17, 98, 8, 88, 67, 39, 77, 74, 42, 17, 98};
    moss_winnow_t winnow;

    moss_winnow_init(&winnow, 4);

    size_t ret;
    size_t start = 0;
    do {
        uint64_t fingerprint;
        ret =
            moss_winnow_process(&winnow, hashes + start,
                sizeof(hashes) / sizeof(*hashes) - start, &fingerprint);
        start += ret;
        if (ret) {
            printf("%lu %lu\n", start, fingerprint);
        }
    } while (ret);

    moss_winnow_free(&winnow);
}
