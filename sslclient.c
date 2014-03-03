#define _POSIX_SOURCE
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>

int min(int a, int b)
{
    return a < b ?  a: b;
}

int
servernameCb(SSL *ssl, int *al, void *arg)
{
    int type = SSL_get_servername_type(ssl);
    const char *name = SSL_get_servername_type(ssl);
    fprintf(stderr, "servername callback on SSL %p, context = %s, servername=%s\n", ssl, (char *)arg, name);

    return SSL_TLSEXT_ERR_OK;
}

static void
run(int sock)
{
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_method());
    assert(ctx);
    SSL_CTX_set_tlsext_servername_callback(ctx, servernameCb);
    SSL_CTX_set_tlsext_servername_arg(ctx, "context");
    BIO *bio = BIO_new_fd(sock, 0);
    BIO *sslBio = BIO_new_ssl(ctx, 1);
    BIO_push(sslBio, bio);
    const char msg[] = "GET / HTTP/1.1\r\nHost:localhost\r\n\r\n";
    BIO_write(sslBio, msg, sizeof msg - 1);
    fprintf(stderr, "write: {%s}", msg);
    for (;;) {
        char buf[1024];
        int rc = BIO_read(sslBio, buf, sizeof buf);
        if (rc <= 0)
            break;
        fprintf(stderr, "read %.*s%s\n", min(rc, 30), buf, rc > 30 ? "..." : "");
    }
    BIO_free_all(sslBio);
}

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
    SSL_library_init();
    OPENSSL_add_all_algorithms_conf();
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
        run(sock);
        close(sock);
    }
    freeaddrinfo(ai);
}
