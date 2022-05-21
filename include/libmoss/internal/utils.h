#ifndef LIBMOSS_INTERNAL_UTILS_H
#define LIBMOSS_INTERNAL_UTILS_H

#include <stddef.h>
#include <stdio.h>

char *moss_dirname(char *path);

int moss_jsondump(FILE *stream, const char *buf, size_t len);
int moss_jsondumpstr(FILE *stream, const char *str);
int moss_jsondumpf(FILE *stream, FILE *in);

#endif /* libmoss/internal/utils.h */
