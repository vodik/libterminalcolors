#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <dirent.h>

#ifndef TERMINAL_COLORS
#define TERMINAL_COLORS "/etc/terminal-colors.d/"
#endif

static void parse_support(const char *name, const char *filter)
{
    char *p, *dup = strdup(name);
    size_t len = strlen(name);

    p = memrchr(dup, '.', len);
    const char *type = p ? p + 1 : dup;

    if (strcmp(type, "disable") == 0)
        printf("DISABLE:\n");
    else if (strcmp(type, "enable") == 0)
        printf("ENABLE:\n");
    else if (strcmp(type, "scheme") == 0)
        printf("SCHEME:\n");
    else
        printf("UNKNOWN:\n");

    if (type == dup) {
        printf("  globally\n");
        return;
    }

    // TODO: return lengths instead
    *p = 0;

    p = memchr(dup, '@', p - dup);
    const char *term = p ? p + 1 : NULL;

    if (term) {
        printf("  term: %s\n", term);

        // TODO: return lengths instead
        *p = 0;
    }

    printf("  name: %s\n", dup[0] ? dup : "*");

    free(dup);
}

static void search_support(const char *filter)
{
    int dirfd;
    DIR *dirp;

    dirfd = open(TERMINAL_COLORS, O_RDONLY | O_DIRECTORY);
    if (dirfd < 0)
        err(1, "couldn't access terminal-colors.d");

    dirp = fdopendir(dirfd);
    if (!dirp)
        err(1, "fdopendir failed");

    const struct dirent *dp;
    while ((dp = readdir(dirp))) {
        if (dp->d_type != DT_REG && dp->d_type != DT_UNKNOWN)
            continue;

        parse_support(dp->d_name, filter);
    }
}

int main(int argc, char *argv[])
{
    if (access(TERMINAL_COLORS, R_OK) < 0)
        err(1, "couldn't access terminal-colors.d");

    // TODO: disable can be overridden by an explicit enable
    int test = access(TERMINAL_COLORS "disable", R_OK);
    if (test == 0) {
        printf("TERMINAL COLORS DISABLED!\n");
        return 0;
    } else if (test < 0 && errno != ENOENT) {
        err(1, "couldn't access terminal-colors.d/disable");
    }

    search_support("dmesg");
}
