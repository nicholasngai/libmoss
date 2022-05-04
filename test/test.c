#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libmoss/hashing.h>
#include <libmoss/winnowing.h>

static void test_hashing(void) {
    static const uint64_t tokens[] = {
        77, 74, 42, 17, 98, 50, 17, 98, 8, 88, 67, 39, 77, 74, 42, 17, 98
    };
    static const uint64_t expected[] = {
        210681571385, 210676839038, 210638077733, 210611287325, 210705588596,
        210647563786, 210611243899, 210704155569, 210600273884, 210694363646,
        210668494320, 210636649790, 210681571385,
    };
    uint64_t result[13];
    size_t result_len = 0;
    moss_hashing_t hashing;
    int ret;

    ret = moss_hashing_init(&hashing, 5);
    assert(!ret);

    moss_hashing_input_tokens(&hashing, tokens,
            sizeof(tokens) / sizeof(*tokens));

    while (result_len < sizeof(result) / sizeof(*result)) {
        ret = moss_hashing_get_hashes(&hashing, result + result_len,
                sizeof(result) / sizeof(*result) - result_len);
        assert(ret > 0);
        result_len += ret;
    }

    /* Check that the future hash call returns 0 since the input buffer should
     * have been consumed. */
    ret = moss_hashing_get_hashes(&hashing, result, 1);
    assert(!ret);

    assert(result_len == 13);
    assert(!memcmp(result, expected, sizeof(result)));

    moss_hashing_free(&hashing);
}

static void test_winnow(void) {
    static const uint64_t hashes[] = {
        77, 74, 42, 17, 98, 50, 17, 98, 8, 88, 67, 39, 77, 74, 42, 17, 98
    };
    static const uint64_t expected[] = { 17, 17, 8, 39, 17 };
    uint64_t result[5];
    size_t result_len = 0;
    moss_winnow_t winnow;
    int ret;

    ret = moss_winnow_init(&winnow, 4);
    assert(!ret);

    size_t hashes_read;
    size_t start = 0;
    do {
        uint64_t fingerprint;
        hashes_read =
            moss_winnow_process(&winnow, hashes + start,
                sizeof(hashes) / sizeof(*hashes) - start, &fingerprint);
        start += hashes_read;
        if (hashes_read) {
            assert(result_len < 5);
            result[result_len] = fingerprint;
            result_len++;
        }
    } while (hashes_read);

    assert(result_len == 5);
    assert(!memcmp(result, expected, sizeof(result)));

    moss_winnow_free(&winnow);
}

int main(void) {
    test_hashing();
    test_winnow();
}
