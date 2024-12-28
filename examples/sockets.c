
#include <stdio.h>

#define HGL_FLAGS_IMPLEMENTATION
#include "hgl_flags.h"

#define HGL_SOCKETS_IMPLEMENTATION
#include "hgl_sockets.h"

char scratch[1000000];
char buf[16];

void tcp_client(const char *host, const char *port)
{
    int sockfd = hgl_socket_client(host, port, HGL_SOCKET_TCP);
    if (sockfd == -1) {
        printf("Error connecting to %s:%s.\n", host, port);
        return;
    }

    for (;;) {
        char *s = fgets(scratch, 1024, stdin);
        write(sockfd, s, strlen(s));
        ssize_t n = read(sockfd, scratch, 1024);
        if (n <= 0) break;
        scratch[n] = '\0';
        printf("%s", scratch);
    }

    close(sockfd);
}

void udp_client(const char *host, const char *port)
{
    int sockfd = hgl_socket_client(host, port, HGL_SOCKET_UDP);
    if (sockfd == -1) {
        printf("Error %s:%s.\n", host, port);
        return;
    }

    for (;;) {
        char *s = fgets(scratch, 1024, stdin);
        write(sockfd, s, strlen(s));
        ssize_t n = read(sockfd, scratch, 1024);
        if (n <= 0) break;
        scratch[n] = '\0';
        printf("%s", scratch);
    }

    close(sockfd);
}

void tcp_echo_server(const char *port)
{
    int server = hgl_socket_server(port, HGL_SOCKET_TCP);
    if (server == -1) {
        printf("Error creating socket server on port %s.\n", port);
        return;
    }

    int client = hgl_socket_accept(server);

    for (;;) {
        ssize_t n = read(client, scratch, 1024);
        if (n <= 0) break;
        scratch[n] = '\0';
        write(client, scratch, n);
    }

    close(client);
    close(server);
}

void udp_echo_server(const char *port)
{
    int sockfd = hgl_socket_server(port, HGL_SOCKET_UDP);
    if (sockfd == -1) {
        printf("Error creating socket server on port %s.\n", port);
        return;
    }

    for (;;) {
        struct sockaddr src_addr;
        socklen_t src_addr_len = sizeof src_addr;
        ssize_t n = recvfrom(sockfd, scratch, 1024, 0, &src_addr, &src_addr_len);
        if (n <= 0) break;
        scratch[n] = '\0';
        n = sendto(sockfd, scratch, n, 0, &src_addr, src_addr_len);
    }

    close(sockfd);
}

int main(int argc, char *argv[])
{

    bool *opt_tcp_client = hgl_flags_add_bool("--tcp-client", "netcat style tcp client", false, 0);
    bool *opt_udp_client = hgl_flags_add_bool("--udp-client", "netcat style udp client", false, 0);
    bool *opt_tcp_server = hgl_flags_add_bool("--tcp-server", "TCP echo server", false, 0);
    bool *opt_udp_server = hgl_flags_add_bool("--udp-server", "UDP echo server", false, 0);
    const char **opt_host = hgl_flags_add_str("-h,--host", "host", "127.0.0.1", 0);
    const char **opt_port = hgl_flags_add_str("-p,--port", "port", "6969", 0);

    int err = hgl_flags_parse(argc, argv);
    if (err != 0) {
        printf("Usage: %s [options]\n", argv[0]);
        hgl_flags_print();
        return 1;
    }

    if (*opt_tcp_client) {
        tcp_client(*opt_host, *opt_port);
    } else if (*opt_udp_client) {
        udp_client(*opt_host, *opt_port);
    } else if (*opt_tcp_server) {
        tcp_echo_server(*opt_port);
    } else if (*opt_udp_server) {
        udp_echo_server(*opt_port);
    } else {
        printf("Usage: %s [options]\n", argv[0]);
        hgl_flags_print();
        return 0;
    }

    return 0;

#if 0
    memset(buf, 'a', sizeof(buf));

    //int sockfd = hgl_socket_client("localhost", "6969", HGL_SOCKET_TCP);
    int sockfd = hgl_socket_client("127.0.0.1", "6969", HGL_SOCKET_TCP);
    if (sockfd == -1) {
        printf("Error connecting\n");
        return 1;
    }
    printf("sockfd = %d\n", sockfd);
    write(sockfd, buf, sizeof(buf));

    close(sockfd);
#elif 0 
    int server = hgl_socket_server("6969", HGL_SOCKET_TCP);
    if (server == -1) {
        printf("Error A \n");
        return 1;
    }
    int client = hgl_socket_accept(server);

    for (;;) {
        ssize_t n_read = read(client, scratch, 4096);
        if (n_read == -1) {
            printf("A\n");
            break;
        } else if (n_read == 0) {
            printf("B\n");
            break;
        } 
        printf("%s", scratch);
        memset(scratch, 0, 4096);
    }

    close(client);
#else
    int sockfd = hgl_socket_server("6969", HGL_SOCKET_UDP);
    if (sockfd == -1) {
        printf("Error A \n");
        return 1;
    }

    for (;;) {
        //ssize_t n_read = recv(sockfd, scratch, 4096, 0);
        ssize_t n_read = read(sockfd, scratch, 4096);
        if (n_read == -1) {
            printf("A\n");
            break;
        } else if (n_read == 0) {
            printf("B\n");
            break;
        } 
        printf("%s", scratch);
        memset(scratch, 0, 4096);
    }

#endif


}
