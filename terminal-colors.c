#include <unistd.h>
#include <errno.h>
#include <err.h>

int main(int argc, char *argv[])
{
    if (access("/etc/terminal-colors.d/", R_OK) < 0)
        err(1, "couldn't access terminal-colors.d");

    /* const char *program = argv[1]; */
    /* if (argc != 2) */
    /*     return -1; */
}
