
#include "hgl_test.h"

#define HGL_SOCKETS_IMPLEMENTATION
#include "hgl_sockets.h"

#include <pthread.h>
#include <semaphore.h>

char scratch[1024];

sem_t sem;

void *tcp_echo_server(void *args)
{
    (void) args;
    int server_socket = hgl_socket_server("6969", HGL_SOCKET_TCP);
    ASSERT(server_socket != -1);
    sem_post(&sem);
    int client_socket = hgl_socket_accept(server_socket);

    for (;;) {
        ssize_t n = read(client_socket, scratch, 1024);
        if (n <= 0) break;
        scratch[n] = '\0';
        write(client_socket, scratch, n);
    }

    close(client_socket);
    close(server_socket);
    return NULL;
}

TEST(tcp_client_server, .input = "Hello!\n", .expect_output = "Hello!\n")
{
    ASSERT(sem_init(&sem, 0, 0) == 0);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, tcp_echo_server, NULL);

    sem_wait(&sem);

    int sockfd = hgl_socket_client("127.0.0.1", "6969", HGL_SOCKET_TCP);
    ASSERT(sockfd != -1);

    char *s = fgets(scratch, 1024, stdin);
    write(sockfd, s, strlen(s));
    ssize_t n = read(sockfd, scratch, 1024);
    ASSERT(n > 0);
    scratch[n] = '\0';
    printf("%s", scratch);

    close(sockfd);

    pthread_join(server_thread, NULL);
    sem_close(&sem);
}

void *udp_echo_server(void *args)
{
    (void) args;
    int sockfd = hgl_socket_server("6969", HGL_SOCKET_UDP);
    ASSERT(sockfd != -1);
    sem_post(&sem);

    struct sockaddr src_addr;
    socklen_t src_addr_len = sizeof src_addr;
    ssize_t n = recvfrom(sockfd, scratch, 1024, 0, &src_addr, &src_addr_len);
    ASSERT(n > 0);
    scratch[n] = '\0';
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&src_addr;
    char *s = inet_ntoa(addr_in->sin_addr);
    n = sendto(sockfd, scratch, n, 0, &src_addr, src_addr_len);

    close(sockfd);
    return NULL;
}

TEST(udp_client_server, .input = "Kalas!\n", .expect_output = "Kalas!\n")
{
    ASSERT(sem_init(&sem, 0, 0) == 0);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, udp_echo_server, NULL);

    sem_wait(&sem);

    int sockfd = hgl_socket_client("127.0.0.1", "6969", HGL_SOCKET_UDP);
    ASSERT(sockfd != -1);

    char *s = fgets(scratch, 1024, stdin);
    write(sockfd, s, strlen(s));
    ssize_t n = read(sockfd, scratch, 1024);
    ASSERT(n > 0);
    scratch[n] = '\0';
    printf("%s", scratch);

    close(sockfd);

    pthread_join(server_thread, NULL);
    sem_close(&sem);
}
