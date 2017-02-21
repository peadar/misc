#define _POSIX_SOURCE
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

int
main(int argc, char *argv[])
{
    int rc, addrcount, sock;
    struct sockaddr_storage storage;
    struct addrinfo *ai, *aip, hints;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; /* just get IPv4, which we know is smaller than storage. */
    hints.ai_socktype = SOCK_STREAM;

    rc = getaddrinfo(argv[1], argv[2], &hints, &ai);

    if (rc != 0) {
        fprintf(stderr, "can't resolve address %s:%s : %s\n", argv[1], argv[2], gai_strerror(rc));
        exit(1);
    }

    for (aip = ai, addrcount = 0; aip; aip = aip->ai_next, ++addrcount) {
        assert(aip->ai_addrlen <= sizeof storage);
        memset(&storage, 0, sizeof storage);
        memcpy(&storage, aip->ai_addr, aip->ai_addrlen);
        sock = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
        if (sock == -1) {
            fprintf(stderr, "can't connect to address %d\n", addrcount);
            continue;
        }
        rc = connect(sock, (const struct sockaddr *)&storage, sizeof storage);
        if (rc == 0)
            printf("connect: success\n");
        else
            printf("connect: fail: %s\n", strerror(errno));
        close(sock);
    }
    freeaddrinfo(ai);
}
