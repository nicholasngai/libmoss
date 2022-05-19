#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libmoss/moss.h>
#include "libmoss/internal/utils.h"

enum moss_language {
    LANG_NONE = 0,
    LANG_GO,
};

moss_doc_t **all_docs;
size_t all_docs_len;
size_t all_docs_cap;

static void show_help(char *name) {
    printf("usage: %s -lLANGUAGE FILE [FILE...]\n", name);
}

static moss_doc_t *alloc_doc(int64_t doc_id, char *filename) {
    /* Allocate document. */
    moss_doc_t *doc = malloc(sizeof(*doc));
    if (!doc) {
        perror("Error allocating document");
        goto exit;
    }
    doc->id = doc_id;
    doc->path = filename;

    /* Extend array if needed. */
    if (all_docs_len == all_docs_cap) {
        size_t new_cap = all_docs_cap ? all_docs_cap * 2 : 256;
        moss_doc_t **new_all_docs =
            realloc(all_docs, new_cap * sizeof(moss_doc_t *));
        if (!new_all_docs) {
            perror("Error extending doc array");
            goto exit_free_doc;
        }
        all_docs = new_all_docs;
        all_docs_cap = new_cap;
    }

    all_docs_len += 1;
    all_docs[all_docs_len - 1] = doc;

    return doc;

exit_free_doc:
    free(doc);
exit:
    return NULL;
}

static void free_docs(void) {
    for (size_t i = 0; i < all_docs_len; i++) {
        free(all_docs[i]);
    }
    free(all_docs);
    all_docs = NULL;
    all_docs_len = 0;
    all_docs_cap = 0;
}

static int process_doc(moss_t *moss, char *parser_path,
        int64_t doc_id, char *filename) {
    int ret;

    /* Open parser. */
    int fds[2];
    ret = pipe(fds);
    if (ret) {
        goto exit;
    }
    pid_t child = fork();
    if (child < 0) {
        perror("Error forking tokenizer");
        close(fds[1]);
        goto exit_close_pipes;
    }
    if (!child) {
        ret = close(fds[0]);
        if (ret) {
            perror("tokenizer: Error closing read pipe");
            exit(-1);
        }
        ret = dup2(fds[1], STDOUT_FILENO);
        if (ret == -1) {
            perror("tokenizer: Error setting stdout");
            exit(-1);
        }
        ret = close(STDIN_FILENO);
        if (ret) {
            perror("tokenizer: Error closing stdin");
            exit(-1);
        }
        ret = execlp(parser_path, parser_path, filename);
        if (ret) {
            perror("tokenizer: Error executing tokenizer");
            exit(-1);
        }
    } else {
        ret = close(fds[1]);
        if (ret) {
            perror("Error closing write pipe");
            goto exit_kill_child;
        }
    }
    FILE *input = fdopen(fds[0], "r");
    if (!input) {
        perror("Error opening pipe to tokenizer");
        goto exit_kill_child;
    }
    fds[0] = -1;

    moss_doc_t *doc = alloc_doc(doc_id, filename);
    if (!doc) {
        perror("Error allocating document");
        goto exit_close_input;
    }
    moss_token_t token;
    token.doc = doc;
    while (1) {
        /* Get token from stdin. */
        char buf[128];
        if (!fgets(buf, sizeof(buf), input)) {
            break;
        }
        if (buf[0] == '\n') {
            break;
        }
        char *end = buf;
        token.token = strtoull(end, &end, 10);
        if (*end == '\n') {
            token.pos = 0;
        } else {
            if (*end != ' ' || !*(end + 1) || *(end + 1) == '\n') {
                fprintf(stderr, "Invalid token in token stream\n");
                ret = -1;
                goto exit_close_input;
            }
            end++;
            token.pos = strtoul(end, &end, 10);
            if (*end != '\n') {
                fprintf(stderr, "Invalid token in token stream\n");
                ret = -1;
                goto exit_close_input;
            }
        }

        /* Feed token to MOSS. */
        ret = moss_input(moss, doc, &token, 1);
        if (ret) {
            fprintf(stderr, "Error feeding token to MOSS\n");
            goto exit_close_input;
        }
    }

    ret = 0;

exit_close_input:
    fclose(input);
exit_kill_child:
    kill(child, SIGKILL);
    waitpid(child, &ret, 0);
exit_close_pipes:
    if (fds[0] != -1) {
        close(fds[0]);
    }
exit:
    return ret;
}

int main(int argc, char **argv) {
    enum moss_language language = LANG_NONE;
    char *language_str;
    char parser_path[PATH_MAX];
    int ret;

    /* Parse options. */
    ret = getopt(argc, argv, "l:");
    switch (ret) {
    case 'l':
        if (!strcmp(optarg, "go")) {
            language = LANG_GO;
        } else {
            fprintf(stderr, "Unknown language: %s\n", optarg);
            return -1;
        }
        language_str = optarg;
        break;
    case '?':
        show_help(argv[0]);
        return -1;
        break;
    }

    if (language == LANG_NONE) {
        show_help(argv[0]);
        return -1;
    }

    /* Load parser. */
    strncpy(parser_path, argv[0], sizeof(parser_path) - 1);
    parser_path[strnlen(parser_path, sizeof(parser_path) - 1)] = '\0';
    moss_dirname(parser_path);
    size_t path_len = strlen(parser_path);
    strncat(parser_path, path_len > 0 ? "/moss_tokenizer_" : "moss_tokenizer_",
            sizeof(parser_path) - path_len - 1);
    path_len += strlen(path_len > 0 ? "/moss_tokenizer_" : "moss_tokenizer_");
    strncat(parser_path, language_str, sizeof(parser_path) - path_len - 1);
    path_len += strlen(language_str);

    /* Initialize MOSS. */
    moss_t moss;
    ret = moss_init(&moss, 16, 64);
    if (ret) {
        fprintf(stderr, "Error initializing MOSS\n");
        goto exit;
    }

    /* Process documents. */
    int64_t cur_doc = 1;
    for (; optind < argc; optind++) {
        ret = process_doc(&moss, parser_path, cur_doc, argv[optind]);
        if (ret) {
            fprintf(stderr, "Error processing doc: %s\n", argv[optind]);
            goto exit_free_moss;
        }
        cur_doc++;
    }

    /* Dump matches. */
    for (struct moss_multimap_iter iter =
            moss_multimap_iter_begin(&moss.fingerprints);
            moss_multimap_iter_finished(&iter);
            moss_multimap_iter_next(&iter)) {
        if (iter.bucket->vals_len == 1) {
            /* Skip non-matches. */
            continue;
        }

        /* Dump all pairs. */
        for (size_t i = 0; i < iter.bucket->vals_len; i++) {
            for (size_t j = i + 1; j < iter.bucket->vals_len; j++) {
                moss_hash_t *entry1 = iter.bucket->vals[i];
                moss_hash_t *entry2 = iter.bucket->vals[j];
                if (entry1->doc == entry2->doc) {
                    continue;
                }
                printf("(%s:%lu-%lu, %s:%lu-%lu)\n",
                        entry1->doc->path, entry1->start_pos, entry1->end_pos,
                        entry2->doc->path, entry2->start_pos, entry2->end_pos);
            }
        }
    }

    free_docs();
exit_free_moss:
    moss_free(&moss);
exit:
    return ret;
}
