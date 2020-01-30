#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
    int i;

    if (argc == 1) {
        for (i = 0; i < ERANGE; ++i) {
            printf("%d\t%s\n", i, strerror(i));
        }
        return 0;
    }

    for (i = 1; i < argc; ++i) {
        char *p = 0;
        long x = strtol(argv[i], &p, 0);
        if (*p != 0)
            fprintf(stderr, "not a number: %s\n", argv[i]);
        else
            fprintf(stdout, "%ld: %s\n", x, strerror(x));
    }
}
