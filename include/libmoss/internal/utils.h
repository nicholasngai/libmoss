#ifndef LIBMOSS_INTERNAL_UTILS_H
#define LIBMOSS_INTERNAL_UTILS_H

#include <string.h>
#include <stdio.h>

static inline char *moss_dirname(char *path) {
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

static inline void moss_fjsondump(FILE *stream, const char *str) {
    while (*str) {
        switch (*str) {
        case '"':
            fputs("\\\"", stream);
            break;
        case '\\':
            fputs("\\\\", stream);
            break;
        case '\x08':
            fputs("\\b", stream);
            break;
        case '\x0c':
            fputs("\\f", stream);
            break;
        case '\x0a':
            fputs("\\n", stream);
            break;
        case '\x0d':
            fputs("\\r", stream);
            break;
        case '\x09':
            fputs("\\t", stream);
            break;
        default:
            if ((unsigned char) *str <= 0x1f) {
                fprintf(stream, "\\%04x", *str);
            } else {
                fputc(*str, stream);
            }
            break;
        }
        str++;
    }
}

#endif /* libmoss/internal/utils.h */
