#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <dirent.h>

int main(int argc, char *argv[])
{

    if (access("/etc/terminal-colors.d/", R_OK) < 0)
        err(1, "couldn't access terminal-colors.d");

    int test = access("/etc/terminal-colors.d/disable", R_OK);
    if (test == 0) {
        printf("TERMINAL COLORS DISABLED!\n");
        return 0;
    } else if (test < 0 && errno != ENOENT) {
        err(1, "couldn't access terminal-colors.d/disable");
    }

    int dirfd;
    DIR *dirp;

    dirfd = open("/etc/terminal-colors.d/", O_RDONLY | O_DIRECTORY);
    dirp = fdopendir(dirfd);
    if (!dirp)
        err(1, "fdopendir failed");

    const struct dirent *dp;
    while ((dp = readdir(dirp))) {
        if (dp->d_type != DT_REG && dp->d_type != DT_UNKNOWN)
            continue;

        printf("found: %s\n", dp->d_name);
    }
}
