#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libmoss/hashing.h>
#include <libmoss/internal/multimap.h>
#include <libmoss/moss.h>
#include <libmoss/winnowing.h>

static void test_multimap(void) {
    int ret;

    struct moss_multimap multimap;
    ret = moss_multimap_init(&multimap, 123);
    assert(!ret);

    for (uint64_t i = 0; i < 256; i++) {
        ret = moss_multimap_get(&multimap, i, NULL, NULL);
        assert(!ret);
    }

    for (uint64_t i = 0; i < 256; i++) {
        ret = moss_multimap_add(&multimap, i, (void *) (i + 100));
        assert(!ret);
    }

    for (uint64_t i = 0; i < 256; i++) {
        ret = moss_multimap_add(&multimap, i, (void *) (i + 200));
        assert(!ret);
    }

    for (uint64_t i = 0; i < 256; i++) {
        uint64_t *vals;
        size_t vals_len;
        ret = moss_multimap_get(&multimap, i, (void *) &vals, &vals_len);
        assert(ret);
        assert(vals_len == 2);
        assert(vals[0] == i + 100);
        assert(vals[1] == i + 200);
    }

    moss_multimap_free(&multimap, NULL);
}

static void test_hashing(void) {
    static moss_doc_t doc = {
        .id = 1,
        .path = "/path/to/doc",
    };
    static const moss_token_t tokens[] = {
        { .token = 77, .doc = &doc },
        { .token = 74, .doc = &doc },
        { .token = 42, .doc = &doc },
        { .token = 17, .doc = &doc },
        { .token = 98, .doc = &doc },
        { .token = 50, .doc = &doc },
        { .token = 17, .doc = &doc },
        { .token = 98, .doc = &doc },
        { .token = 8, .doc = &doc },
        { .token = 88, .doc = &doc },
        { .token = 67, .doc = &doc },
        { .token = 39, .doc = &doc },
        { .token = 77, .doc = &doc },
        { .token = 74, .doc = &doc },
        { .token = 42, .doc = &doc },
        { .token = 17, .doc = &doc },
        { .token = 98, .doc = &doc },
    };
    static const moss_hash_t expected[] = {
        { .hash = 210681571385, .doc = &doc },
        { .hash = 210676839038, .doc = &doc },
        { .hash = 210638077733, .doc = &doc },
        { .hash = 210611287325, .doc = &doc },
        { .hash = 210705588596, .doc = &doc },
        { .hash = 210647563786, .doc = &doc },
        { .hash = 210611243899, .doc = &doc },
        { .hash = 210704155569, .doc = &doc },
        { .hash = 210600273884, .doc = &doc },
        { .hash = 210694363646, .doc = &doc },
        { .hash = 210668494320, .doc = &doc },
        { .hash = 210636649790, .doc = &doc },
    };
    moss_hash_t result[12];
    size_t result_len = 0;
    moss_hashing_t hashing;
    int ret;

    ret = moss_hashing_init(&hashing, 5);
    assert(!ret);

    moss_hashing_input_tokens(&hashing, tokens, 2);

    do {
        ret = moss_hashing_get_hashes(&hashing, result + result_len,
                sizeof(result) / sizeof(*result) - result_len);
        assert(ret >= 0);
        result_len += ret;
    } while (ret > 0 && result_len < 12);

    moss_hashing_input_tokens(&hashing, tokens + 2, 4);

    do {
        ret = moss_hashing_get_hashes(&hashing, result + result_len,
                sizeof(result) / sizeof(*result) - result_len);
        assert(ret >= 0);
        result_len += ret;
    } while (ret > 0 && result_len < 12);

    moss_hashing_input_tokens(&hashing, tokens + 6, 9);

    do {
        ret = moss_hashing_get_hashes(&hashing, result + result_len,
                sizeof(result) / sizeof(*result) - result_len);
        assert(ret >= 0);
        result_len += ret;
    } while (ret > 0 && result_len < 12);

    moss_hashing_input_tokens(&hashing, tokens + 15,
            sizeof(tokens) / sizeof(*tokens) - 15);

    do {
        ret = moss_hashing_get_hashes(&hashing, result + result_len,
                sizeof(result) / sizeof(*result) - result_len);
        assert(ret >= 0);
        result_len += ret;
    } while (ret > 0 && result_len < 12);

    assert(result_len == 12);

    /* Check that the future hash call returns 0 since the input buffer should
     * have been consumed. */
    ret = moss_hashing_get_hashes(&hashing, result, 1);
    assert(!ret);

    assert(result_len == 12);
    assert(!memcmp(result, expected, sizeof(result)));

    moss_hashing_free(&hashing);
}

static void test_winnow(void) {
    static moss_doc_t doc = {
        .id = 1,
        .path = "/path/to/doc",
    };
    static const moss_hash_t hashes[] = {
        { .hash = 77, .doc = &doc },
        { .hash = 74, .doc = &doc },
        { .hash = 42, .doc = &doc },
        { .hash = 17, .doc = &doc },
        { .hash = 98, .doc = &doc },
        { .hash = 50, .doc = &doc },
        { .hash = 17, .doc = &doc },
        { .hash = 98, .doc = &doc },
        { .hash = 8, .doc = &doc },
        { .hash = 88, .doc = &doc },
        { .hash = 67, .doc = &doc },
        { .hash = 39, .doc = &doc },
        { .hash = 77, .doc = &doc },
        { .hash = 74, .doc = &doc },
        { .hash = 42, .doc = &doc },
        { .hash = 17, .doc = &doc },
        { .hash = 98, .doc = &doc },
    };
    static const moss_hash_t expected[] = {
        { .hash = 17, .doc = &doc },
        { .hash = 17, .doc = &doc },
        { .hash = 8, .doc = &doc },
        { .hash = 39, .doc = &doc },
        { .hash = 17, .doc = &doc },
    };
    moss_hash_t result[5];
    size_t result_len = 0;
    moss_winnow_t winnow;
    int ret;

    ret = moss_winnow_init(&winnow, 4);
    assert(!ret);

    size_t hashes_read;
    size_t start = 0;
    do {
        moss_hash_t fingerprint;
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

static void test_moss(void) {
    int ret;

    moss_t moss;
    ret = moss_init(&moss, 16, 64);
    assert(!ret);

    moss_free(&moss);
}

int main(void) {
    test_multimap();
    test_hashing();
    test_winnow();
    test_moss();
}
