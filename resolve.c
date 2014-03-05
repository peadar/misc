// compile as:  cc -std=c99 -o resolve resolve.c

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <err.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *res;

    const char *hostname = 0;
    const char *port = "http";

    switch (argc) {
	default: errx(1, "usage: test <host> [port]");
	case 3: port = argv[2];
	case 2: hostname = argv[1];
    }

    int rc;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    for (;;) {
	struct timeval start, end;
	gettimeofday(&start, 0);
        rc = getaddrinfo(hostname, port, &hints, &res);
	gettimeofday(&end, 0);
	suseconds_t msec = (end.tv_sec - start.tv_sec) * 1000;
	msec += end.tv_usec / 1000;
	msec -= start.tv_usec / 1000;
	printf("elapsed: %dms. current time: %s", (int)msec, ctime(&end.tv_sec));
        if (rc != 0) {
            printf("\tfailed to resolve: %s/%s", gai_strerror(rc), strerror(errno));
	} else {
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
		    printf("\t%s:%s\n", hostname, portname);
		else
		    printf("\t(failed to resolve address)\n");
	    }
	}
        printf("\n");
        sleep(2);
    }
}

