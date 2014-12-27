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

static void parse_support(const char *name)
{
    char *dup = strdup(name);
    size_t len = strlen(name);

    size_t at = strcspn(name, "@");
    size_t dot = strcspn(name, ".");

    dup[at] = 0;
    dup[dot] = 0;

    // FIXME: memory overflow
    printf("bin:    %s\n", dup);
    if (at != len)
        printf("term:   %s\n", &dup[at + 1]);
    if (dot != len)
        printf("status: %s\n", &dup[dot + 1]);
    printf("\n");
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

        parse_support(dp->d_name);
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

    search_support(NULL);
}
