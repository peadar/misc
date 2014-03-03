#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <err.h>

int
main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *res;

    if (argc != 3) {
        errx(1, "usage: test <host> <port>");
    }
    int rc;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    for (;;) {
        rc = getaddrinfo(argv[1], argv[2], &hints, &res);
        if (rc != 0)
            errx(1, "failed to resolve: %s", gai_strerror(rc));
        for (struct addrinfo *resp = res; resp; resp = resp->ai_next) {
            char hostname[1025];
            char portname[33];
            rc = getnameinfo(resp->ai_addr,
                resp->ai_addrlen,
                hostname,
                sizeof hostname,
                portname,
                sizeof portname,
                NI_NUMERICSERV | NI_NUMERICHOST);
            if (rc == 0)
                printf("%s:%s\n", hostname, portname);
            else
                printf("(failed to resolve address)\n");
        }
        printf("\n");
        sleep(2);
    }
}

