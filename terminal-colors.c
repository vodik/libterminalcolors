#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <dirent.h>

#ifndef TERMINAL_COLORS
#define TERMINAL_COLORS "/etc/terminal-colors.d/"
#endif

enum {
    COLORS_UNDEF = -1,
    COLORS_AUTO = 0,
    COLORS_NEVER,
    COLORS_ALWAYS,
};

static inline int max(int a, int b)
{
     return a > b ? a : b;
}

/*
* The terminal-colors.d/ evaluation is based on "scores":
*
* filename score
* ---------------------------------------
* type 1
* @termname.type 10 + 1
* utilname.type 20 + 1
* utilname@termname.type 20 + 10 + 1
*
* the match with higher score wins. The score is per type.
*/
struct term_score {
    int enable;
    int disable;
};

static void colors_parse_filename(const char *name, const char *filter, struct term_score *score)
{
    char *p;
    size_t len = strlen(name);

    p = memrchr(name, '.', len);
    const char *type = p ? p + 1 : name;
    int *field = NULL;
    int calc = 1;

    printf("\n> parsing '%s'\n", name);
    if (strcmp(type, "disable") == 0) {
        printf("DISABLE:\n");
        field = &score->disable;
    } else if (strcmp(type, "enable") == 0) {
        printf("ENABLE:\n");
        field = &score->enable;
    } else if (strcmp(type, "scheme") == 0) {
        printf("SCHEME:\n");
        return;
    } else {
        printf("UNKNOWN:\n");
        return;
    }

    if (type == name) {
        printf("  globally\n");
    } else {
        p = memchr(name, '@', type - name - 1);
        const char *term = p ? p + 1 : NULL;

        size_t name_len = (term ? term : type) - name - 1;

        /*{{{ DEBUGGING*/
        if (term) {
            size_t term_len = type - term - 1;
            if (strncmp(term, getenv("TERM"), term_len) == 0) {
                printf("TERM MATCH!\n");
                calc += 10;
            }
            printf("  term: %.*s\n", (int)term_len, term);
        }

        if (name_len) {
            if (strncmp(name, filter, name_len) == 0) {
                printf("NAME MATCH!\n");
                calc += 20;
            } else {
                calc = 0;
            }

            printf("  name: %.*s\n", (int)name_len, name);
        } else
            printf("  name: *\n");
        /*}}}*/
    }

    printf("COMPARING %d vs %d: score %d\n", *field, calc, max(*field, calc));
    *field = max(*field, calc);
}

static int colors_readdir(const char *path, const char *name)
{
    int dirfd;
    DIR *dirp;

    dirfd = open(path, O_RDONLY | O_DIRECTORY);
    if (dirfd < 0)
        err(1, "couldn't access terminal-colors.d");

    dirp = fdopendir(dirfd);
    if (!dirp)
        err(1, "fdopendir failed");

    struct term_score score = {};
    const struct dirent *dp;
    while ((dp = readdir(dirp))) {
        if (dp->d_type != DT_REG && dp->d_type != DT_LNK && dp->d_type != DT_UNKNOWN)
            continue;

        colors_parse_filename(dp->d_name, name, &score);
    }

    printf("\nRESULTS FOR %s\n", name);
    printf("  enable: %d\n", score.enable);
    printf("  disable: %d\n", score.disable);

    return score.disable > score.enable ? COLORS_NEVER: COLORS_AUTO;
}

static bool colors_init(int mode, const char *name)
{
    int atty = -1;

    if (mode == COLORS_UNDEF && isatty(STDOUT_FILENO)) {
        printf("IS A TTY, WILL CONSIDER ENABLE\n");
        mode = colors_readdir(TERMINAL_COLORS, name);
    }

    switch (mode) {
    case COLORS_AUTO:
        return atty == -1 ? isatty(STDOUT_FILENO) : atty;
    case COLORS_ALWAYS:
        return true;
    case COLORS_NEVER:
    default:
        return false;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "%s [program name]\n", program_invocation_short_name);
        return 1;
    }

    int mode = COLORS_UNDEF;
    const char *name = argv[1];

    if (colors_init(mode, name))
        printf("COLORS ENABLED\n");
    else
        printf("COLORS DISABLED\n");
}
