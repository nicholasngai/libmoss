#include "libmoss/internal/utils.h"
#include <string.h>
#include <stdio.h>

char *moss_dirname(char *path) {
    char *end = path + strlen(path);

    /* Strip trailing slashes. */
    while (end > path && *(end - 1) == '/') {
        end--;
    }

    /* Strip the last path component. */
    while (end > path && *(end - 1) != '/') {
        end--;
    }

    /* Strip the remaining slashes. */
    while (end > path && *(end - 1) == '/') {
        end--;
    }

    *end = '\0';
    return path;
}

int moss_jsondump(FILE *stream, const char *buf, size_t len) {
    int ret = 0;

    for (size_t i = 0; i < len; i++) {
        switch (buf[i]) {
        case '"':
            ret = fputs("\\\"", stream);
            break;
        case '\\':
            ret = fputs("\\\\", stream);
            break;
        case '\x08':
            ret = fputs("\\b", stream);
            break;
        case '\x0c':
            ret = fputs("\\f", stream);
            break;
        case '\x0a':
            ret = fputs("\\n", stream);
            break;
        case '\x0d':
            ret = fputs("\\r", stream);
            break;
        case '\x09':
            ret = fputs("\\t", stream);
            break;
        default:
            if ((unsigned char) buf[i] <= 0x1f) {
                ret = fprintf(stream, "\\uj%04x", buf[i]);
            } else {
                ret = fputc(buf[i], stream);
            }
            break;
        }
        if (ret == EOF) {
            ret = -1;
            goto exit;
        }
    }

exit:
    return ret;
}

int moss_jsondumpstr(FILE *stream, const char *str) {
    return moss_jsondump(stream, str, strlen(str));
}

int moss_jsondumpf(FILE *stream, FILE *in) {
    char buf[4096];
    int ret;

    while ((ret = fread(buf, 1, sizeof(buf), in))) {
        if (ret < 0) {
            goto exit;
        }
        size_t bytes_read = ret;
        ret = moss_jsondump(stream, buf, bytes_read);
    }

exit:
    return ret;
}
