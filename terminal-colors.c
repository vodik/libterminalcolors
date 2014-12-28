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

enum {
    COLORS_UNDEF = -1,
    COLORS_AUTO = 0,
    COLORS_NEVER,
    COLORS_ALWAYS,
};

static void parse_support(const char *name, const char *filter)
{
    char *p;
    size_t len = strlen(name);

    p = memrchr(name, '.', len);
    const char *type = p ? p + 1 : name;

    printf("\n> parsing %s\n", name);
    if (strcmp(type, "disable") == 0)
        printf("DISABLE:\n");
    else if (strcmp(type, "enable") == 0)
        printf("ENABLE:\n");
    else if (strcmp(type, "scheme") == 0)
        printf("SCHEME:\n");
    else
        printf("UNKNOWN:\n");

    if (type == name) {
        printf("  globally\n");
        return;
    }

    p = memchr(name, '@', type - name - 1);
    const char *term = p ? p + 1 : NULL;

    if (term) {
        printf("  term: %.*s\n", type - term - 1, term);
    }


    size_t name_len = (term ? term : type) - name - 1;
    if (name_len)
        printf("  name: %.*s\n", name_len, name);
    else
        printf("  name: *\n");
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
        if (dp->d_type != DT_REG && dp->d_type != DT_LNK && dp->d_type != DT_UNKNOWN)
            continue;

        parse_support(dp->d_name, filter);
    }
}

static void colors_init(int mode, const char *name)
{
    int atty = -1;

    if (mode == COLORS_UNDEF && isatty(STDOUT_FILENO)) {
        printf("IS A TTY, WILL CONSIDER ENABLE\n");
    }

    if (access(TERMINAL_COLORS, R_OK) < 0)
        err(1, "couldn't access terminal-colors.d");

    // TODO: disable can be overridden by an explicit enable
    int test = access(TERMINAL_COLORS "disable", R_OK);
    if (test == 0) {
        printf("TERMINAL COLORS DISABLED!\n");
        return;
    } else if (test < 0 && errno != ENOENT) {
        err(1, "couldn't access terminal-colors.d/disable");
    }

    search_support(name);
}

int main(int argc, char *argv[])
{
    int mode = COLORS_UNDEF;
    const char *name = program_invocation_short_name;

    colors_init(mode, name);
}
