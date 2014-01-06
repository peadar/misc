#include <iostream>
#include <sstream>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

static int
getsock(addrinfo *ai)
{
    int addrcount;
    addrinfo *aip;
    for (aip = ai, addrcount = 0; aip; aip = aip->ai_next, ++addrcount) {
        int sock = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
        if (sock == -1) {
            std::clog << addrcount << ": can't create socket: " << strerror(errno) << std::endl;
            continue;
        }
        int rc = connect(sock, (const struct sockaddr *)aip->ai_addr, aip->ai_addrlen);
        if (rc == 0)
            return sock;
    }
    return -1;
}

int
main(int argc, char *argv[])
{
    struct addrinfo *ai, hints;

    if (argc != 4) {
        std::clog << "usage: " << argv[0] << "<host> <port> <path>" << std::endl;
        exit(1);
    }

    const char *host = argv[1];
    const char *port = argv[2];
    const char *path = argv[3];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = 0;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(host, port, &hints, &ai);

    if (rc != 0) {
        std::clog << "can't resolve address " << host <<":" << port << ": " << gai_strerror(rc) << std::endl;
        exit(1);
    }

    int sock = getsock(ai);
    freeaddrinfo(ai);
    if (sock == -1) {
        std::clog << "no usable address for " << host <<":" << port << std::endl;
        exit(1);
    }

    std::ostringstream os;
    os << "GET " << path << " HTTP/1.1\r\n"
        << "Host: localhost\r\n"
        << "\r\n";
    std::string s = os.str();
    if (size_t(write(sock, s.data(), s.size())) != s.size()) {
        std::clog << "write failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    sleep(1);

    struct linger l;
    l.l_onoff = 1;
    l.l_linger = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, &l, sizeof l) != 0) {
        std::clog << "set linger failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    if (write(sock,"x", 1) != 1)
        std::clog << "reset seed write failed: " << strerror(errno) << std::endl;
    if (shutdown(sock, SHUT_RD|SHUT_WR) != 0)
        std::clog << "shutdown failed: " << strerror(errno) << std::endl;
    if (close(sock) != 0)
        std::clog << "close failed: " << strerror(errno) << std::endl;
    exit(0);
}
