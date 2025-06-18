/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_sockets.h is a simple to use wrapper around Linux sockets.
 *
 *
 * USAGE:
 *
 * Include hgl_sockets.h like this:
 *
 *     #define HGL_SOCKETS_IMPLEMENTATION
 *     #include "hgl_sockets.h"
 *
 *
 * EXAMPLE:
 *
 * Note: These examples omit error handling. See the examples/ directory for a
 *       more complete example.
 *
 * tcp client:
 *
 *     int sockfd = hgl_socket_client("localhost", "6969", HGL_SOCKET_TCP);
 *     write(sockfd, buf, 64);
 *     close(sockfd);
 *
 * tcp server:
 *
 *     int server_sockfd = hgl_socket_server("6969", HGL_SOCKET_TCP);
 *     int cliend_sockfd = hgl_socket_accept(server_sockfd);
 *     for (;;) {
 *         ssize_t n = read(cliend_sockfd, buf, 64);
 *         if (n <= 0) break;
 *     }
 *     close(client_sockfd);
 *     close(server_sockfd);
 *
 * udp client:
 *
 *     int sockfd = hgl_socket_client("localhost", "6969", HGL_SOCKET_UDP);
 *     write(sockfd, buf, 64);
 *     close(sockfd);
 *
 * udp server:
 *
 *     int sockfd = hgl_socket_server("6969", HGL_SOCKET_UDP);
 *     for (;;) {
 *         ssize_t n = read(cliend_sockfd, buf, 64);
 *         if (n <= 0) break;
 *     }
 *     close(sockfd);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_SOCKETS_H
#define HGL_SOCKETS_H

typedef enum
{
    HGL_SOCKET_TCP,
    HGL_SOCKET_UDP,
} HglSocketProtocol;

int hgl_socket_client(const char *host, const char *service, HglSocketProtocol protocol);
int hgl_socket_server(const char *service, HglSocketProtocol protocol);
int hgl_socket_accept(int server_socket);

#endif /* HGL_SOCKETS_H */

#ifdef HGL_SOCKETS_IMPLEMENTATION

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#ifndef HGL_SOCKET_SERVER_MAX_PENDING_CONNECTIONS
#define HGL_SOCKET_SERVER_MAX_PENDING_CONNECTIONS 128
#endif

int hgl_socket_client(const char *host, const char *service, HglSocketProtocol protocol)
{
    int err;
    int sockfd;
    struct addrinfo hints;
    struct addrinfo *addrs;
    struct addrinfo *ai;
    int socktype = 0;
    int prot = 0;

    switch (protocol) {
        case HGL_SOCKET_TCP: {
            socktype = SOCK_STREAM;
            prot     = IPPROTO_TCP;
        } break;
        case HGL_SOCKET_UDP: {
            socktype = SOCK_DGRAM;
            prot     = IPPROTO_UDP;
        } break;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET; /* use IPv4 */
    hints.ai_socktype = socktype;
    hints.ai_protocol = prot;

    /* lookup address information */
    err = getaddrinfo(host, service, &hints, &addrs);
    if (err != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err));
        return -1;
    }

    /* loop through the results and connect to the first possible option */
    for (ai = addrs; ai != NULL; ai = ai->ai_next) {
        /* attempt to open socket */
        sockfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        /* attempt to connect on socket */
        err = connect(sockfd, ai->ai_addr, ai->ai_addrlen);
        if (err != 0) {
            fprintf(stderr, "errno = %s.\n", strerror(errno));
            close(sockfd);
            continue;
        }

        /* done */
        break;
    }

    freeaddrinfo(addrs);

    if (ai == NULL || sockfd == -1 || err != 0) {
        return -1;
    }

    return sockfd;
}

int hgl_socket_server(const char *service, HglSocketProtocol protocol)
{
    int err;
    int sockfd;
    struct addrinfo hints;
    struct addrinfo *addrs;
    struct addrinfo *ai;
    int socktype = 0;
    int prot = 0;

    switch (protocol) {
        case HGL_SOCKET_TCP: {
            socktype = SOCK_STREAM;
            prot     = IPPROTO_TCP;
        } break;
        case HGL_SOCKET_UDP: {
            socktype = SOCK_DGRAM;
            prot     = IPPROTO_UDP;
        } break;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET; /* use IPv4 */
    hints.ai_socktype = socktype;
    hints.ai_protocol = prot;
    hints.ai_flags    = AI_PASSIVE; /* use my ip */
    
    /* lookup address information */
    err = getaddrinfo(NULL, service, &hints, &addrs);
    if (err != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err));
        return -1;
    }

    /* loop through the results and bind the first possible option */
    for (ai = addrs; ai != NULL; ai = ai->ai_next) {
        /* attempt to open socket */
        sockfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (sockfd == -1) {
            continue;
        }
        
        /* attempt to configure socket options */
        int yes = 1;
        err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
        if (err != 0) {
            fprintf(stderr, "errno = %s.\n", strerror(errno));
            close(sockfd);
            continue;
        }
        
        /* attempt to bind */
        err = bind(sockfd, ai->ai_addr, ai->ai_addrlen);
        if (err != 0) {
            fprintf(stderr, "errno = %s.\n", strerror(errno));
            close(sockfd);
            continue;
        }

        /* done */
        break;
    }
    freeaddrinfo(addrs);

    if (ai == NULL || sockfd == -1 || err != 0) {
        return -1;
    }

    if (protocol == HGL_SOCKET_UDP) {
        return sockfd;
    }

    /* TCP: make socket listening */
    if (listen(sockfd, HGL_SOCKET_SERVER_MAX_PENDING_CONNECTIONS) == -1) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int hgl_socket_accept(int server_socket)
{
    socklen_t sin_size = sizeof(struct sockaddr_storage);
    struct sockaddr_storage client_addr;
    int client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &sin_size);
    if (client_socket == -1) {
        fprintf(stderr, "errno = %s.\n", strerror(errno));
        return -1;
    }
    return client_socket;
}

#endif /* HGL_SOCKETS_IMPLEMENTATION */

