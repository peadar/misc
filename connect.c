#define _POSIX_SOURCE
#define _GNU_SOURCE
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

void
usage()
{
    fprintf(stderr, "usage: connect -h <host> -p <port> -c -l <chunk>\n");
    exit(-1);
}

int
expand_crlf(const char *in, char *out, size_t len)
{
    const char *p;
    char *q = out;
    for (p = in; p != in + len;) {
        if (*p == '\n')
            *q++ = '\r';
        *q++ = *p++;
    }
    return q - out;
}

int
main(int argc, char *argv[])
{
    int rc, addrcount, sock;
    struct addrinfo *ai, *aip, hints;
    const char *host = 0;
    const char *port = 0;
    int c;
    int chunklen = 3;
    bool crlf = false;

    while ((c = getopt(argc, argv, "h:p:l:c")) != -1) {
        switch (c) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'l':
                chunklen = atoi(optarg);
                break;
            case 'c':
                crlf = true;
                break;
            default:
                usage();
        }
    }
    if (host == 0 || port == 0)
        usage();
    char *in = malloc(chunklen);
    char *out = malloc(chunklen * 2);


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; /* just get IPv4, which we know is smaller than storage. */
    hints.ai_socktype = SOCK_STREAM;

    rc = getaddrinfo(host, port, &hints, &ai);

    if (rc != 0) {
        fprintf(stderr, "can't resolve address %s:%s : %s\n", host, port, gai_strerror(rc));
        exit(1);
    }

    for (aip = ai, addrcount = 0; aip; aip = aip->ai_next, ++addrcount) {
        sock = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
        if (sock == -1) {
            fprintf(stderr, "can't connect to address %d\n", addrcount);
            continue;
        }

        rc = connect(sock, ai->ai_addr, ai->ai_addrlen);
        if (rc != 0) {
            fprintf(stderr, "connect: fail: %s\n", strerror(errno));
            close(sock);
            continue;
        }
        printf("connected\n");
        if (fork()) {
            for (;;) {
                rc = read(sock, in, chunklen);
                if (rc <= 0)
                    _exit(0);
                printf("<{%.*s}\n", rc, in);
            }
        }
        for (;;) {
            ssize_t rc = read(0, in, chunklen);
            if (rc <= 0)
                break;
            char *w;
            if (crlf) {
                rc = expand_crlf(in, out,  rc);
                w = out;
            } else {
                w = in;
            }
            write(sock, w, rc);
            printf(">{%.*s}\n", (int)rc, w);
            usleep(100000);
        }
        wait(&rc);
        close(sock);
    }
    freeaddrinfo(ai);
}
