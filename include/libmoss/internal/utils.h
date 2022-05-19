#ifndef LIBMOSS_INTERNAL_UTILS_H
#define LIBMOSS_INTERNAL_UTILS_H

#include <string.h>

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

#endif /* libmoss/internal/utils.h */
