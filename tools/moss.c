#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <libmoss/moss.h>

int main(void) {
    int ret;

    /* Initialize MOSS. */
    moss_t moss;
    ret = moss_init(&moss, 16, 64);
    if (ret) {
        fprintf(stderr, "Error initializing MOSS\n");
        goto exit;
    }

    int64_t cur_doc = 1;
    while (1) {
        /* Get token from stdin. */
        uint64_t token;
        char buf[128];
        if (!fgets(buf, sizeof(buf), stdin)) {
            break;
        }
        if (buf[0] == '\n') {
            break;
        }
        char *end;
        token = strtoull(buf, &end, 10);
        if (*end != '\n') {
            fprintf(stderr, "Invalid token in token stream\n");
            ret = -1;
            goto exit_free_moss;
        }
        if (ret == -1) {
            continue;
        }

        /* Feed token to MOSS. */
        ret = moss_input(&moss, cur_doc, &token, 1);
        if (ret) {
            fprintf(stderr, "Error feeding token to MOSS\n");
            goto exit_free_moss;
        }
    }

exit_free_moss:
    moss_free(&moss);
exit:
    return ret;
}
